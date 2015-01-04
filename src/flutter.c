#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>

#include <microhttpd.h>

#include <stdlib.h>
#include <stdio.h>

#include "lua_entry.h"
#include "grim_podalloc.h"

#include "flutter.h"

/* globals */
servlet_t servlet;
struct MHD_Daemon *d;

/* initialize table indexer */
static void init_table_indexer(lua_State *L, table_indexer_t *tab)
{
    lua_newtable(L);
    tab->L = L;
    tab->i = 0;
}

/* build a table of post data on cls */
static int post_iter(void *cls, enum MHD_ValueKind kind, const char *key,
              const char *filename, const char *content_type,
              const char *transfer_encoding, const char *data, uint64_t off,
              size_t size)
{
    const char **tKey, **tValue;
    table_indexer_t *tab = (table_indexer_t *)cls;
    lua_State *L = tab->L;

    const char *tableEntries[] =
    {
        "key",               key,
        "filename",          filename,
        "content_type",      content_type,
        "transfer_encoding", transfer_encoding,
        NULL, NULL
    };

    lua_pushnumber(L, ++tab->i); /* push index of this post data */
    lua_newtable(L);             /* push a new table */

    /* Take care of the null terminated guys succinctly. */
    for (tKey = tableEntries, tValue = tableEntries + 1;
            *tKey != NULL; tKey += 2, tValue += 2)
    {
        /* skip totally empty fields */
        if (*tValue == NULL)
            continue;
        
        /* add an entry in the table */
        lua_pushstring(L, *tKey);
        lua_pushstring(L, *tValue);
        lua_settable(L, -3);
    }

    if (data && off >= 0 && size > 0)
    {
        /* special case for the black sheep: data, which may have embedded
         * null characters and is not guaranteed to be null terminated. */
        lua_pushliteral(L, "data");
        lua_pushlstring(L, data+off, size);
        lua_settable(L, -3);
    }

    /* finish this entry in the post data table */
    lua_settable(L, -3);

    return MHD_YES;
}

static int iter_pairs(void *cls, enum MHD_ValueKind kind, 
                   const char *key, const char *value)
{
    lua_State *L = (lua_State *)cls;

    /* insert the the key value pair in the table at -1 */
    lua_pushstring(L, key);   /* table at -2 */
    lua_pushstring(L, value); /* table at -3 */
    lua_settable(L, -3); /* table is now at -1 after call again */

    return MHD_YES;
}

static void read_pairs(lua_State *L,
        struct MHD_Connection *conn, 
        enum MHD_ValueKind kind)
{
    lua_newtable(L);
    MHD_get_connection_values(conn, kind, &iter_pairs, (void *)L);
}

static int request_opened(void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **con_cls)
{
    int ret;
    servlet_t *servlet = (servlet_t *)cls;
    lua_State *L = servlet->L;
    conn_t *conn = NULL;

    lua_gc(L, LUA_GCSTOP, 0);

    if (*con_cls != NULL)
        conn = (conn_t *)*con_cls;
    else
    {
        conn = GRIM_PODALLOCALLOC(conn_t, &servlet->conn_alloc);

        /* prepare to call onRequest(request) */
        lua_getglobal(L, "onRequest");

        /* initialize building lua tables */
        lua_newtable(L);
        
        /* populate method field */
        lua_pushliteral(L, "method");  /* table at -2 */
        lua_pushstring(L, method);     /* table at -3 */
        lua_settable(L, -3);           /* add entry, table at -1 */

        /* populate url field */
        lua_pushliteral(L, "url");     /* table at -2 */
        lua_pushstring(L, url);        /* table at -3 */
        lua_settable(L, -3);           /* add entry, table at -1 */

        /* populate version field */
        lua_pushliteral(L, "version"); /* table at -2 */
        lua_pushstring(L, version);    /* table at -3 */
        lua_settable(L, -3);           /* add entry, table at -1 */

        /* translate the method to our own literal and a method ID to be used
         * in comparisons from now on.
         *
         * TODO: Recognition can be faster than this.*/
        if (strcmp(method, "GET") == 0)
            conn->method_id = FH1_GET;
        else if (strcmp(method, "POST") == 0)
        {
            conn->method_id = FH1_POST;

            /* Begin building POST table */
            lua_pushliteral(L, "post"); /* push field name */

            init_table_indexer(L, &conn->tab); /* pushes new table */
            conn->posterr   = 0;
            conn->postproc  = MHD_create_post_processor(connection,
                    POSTBUFFERSIZE, post_iter, (void *)&conn->tab);
        }
        /* put and delete last because they are more rare.*/
        else if (strcmp(method, "PUT") == 0)
            conn->method_id = FH1_PUT;
        else if (strcmp(method, "DELETE") == 0)
            conn->method_id = FH1_DELETE;
        else
        {
            fprintf(stderr, "%s%s%s","Bad HTTP request method: ",method,"\n");

            /* Free the connection object. */
            GRIM_PODALLOCFREE(conn_t, &servlet->conn_alloc, conn);
            
            lua_gc(L, LUA_GCRESTART, 0);
            return MHD_NO; /* Close socket without further ado. */
        }

        *con_cls = conn;
    }

    switch (conn->method_id)
    {
    case FH1_POST:
        /* if there's any data left, repeatedly call iter_post. We've
         * initialized the ..., "post": {}, ... part of the table. */
        if (*upload_data_size > 0)
        {
            ret = MHD_post_process(conn->postproc, upload_data,
                        *upload_data_size) != MHD_YES;

            *upload_data_size = 0;

            if (ret != MHD_YES) /* lol whut (you sent me broken http) */
                fprintf(stderr, "%s", "Error processing HTTP POST data!\n");

            lua_gc(L, LUA_GCRESTART, 0);
            return ret;
        }
        /* we're done, finish the post field table. */
        else
            lua_settable(L, -3);

        break;
    case FH1_GET:
        /* Begin building GET table. */
        lua_pushliteral(L, "get");                        /* push field name */
        read_pairs(L, connection, MHD_GET_ARGUMENT_KIND); /* read entries    */

        lua_settable(L, -3);       /* Finished building GET table. */
        break;
    default:
        break;
    };

    /* Begin building header table. */
    lua_pushliteral(L, "head");                 /* push field name */
    read_pairs(L, connection, MHD_HEADER_KIND); /* read entries    */

    lua_settable(L, -3);       /* Finished building header table. */

    /* Begin building footer table. */
    lua_pushliteral(L, "foot");                 /* push field name */
    read_pairs(L, connection, MHD_FOOTER_KIND); /* read entries    */

    lua_settable(L, -3);       /* Finished building footer table. */

    /* call onRequest(request) */
    if (lua_pcall(L, 1, 1, 0))
    {
        fatalreset(servlet->filename, &servlet->L,
                "lua_pcall() to onRequest failed");
        return MHD_NO; /* TODO: should give back HTTP SERVER ERROR */
    }

    /* start reading return value of onRequest() */
    ret = MHD_YES;
    lua_pushnil(L);
    {
        pair_list_t *head = NULL, *foot = NULL;
        struct MHD_Response *resp;
        int answercode = MHD_HTTP_INTERNAL_SERVER_ERROR;
        const char *answerbody = "<ERR>";
        size_t answerlen = 5;
        const char *k, *v;

        while (lua_next(L, -2))
        {
            k = lua_tostring(L, -2);
            if (strcmp(k, "code") == 0)
            {
                if (!lua_isnumber(L, -1))
                {
                    softerrmsg("in return from onRequest() - value of \"code\""
                              " must be a number!");
                }
                else
                    answercode = lua_tonumber(L, -1);
            }
            else if (strcmp(k, "content") == 0)
                if (!lua_isstring(L, -1))
                {
                    softerrmsg("in return from onRequest() - value of"
                              " \"content\" must be a string!");
                }
                else
                    answerbody = lua_tolstring(L, -1, &answerlen);
            else if (strcmp(k, "head") == 0)
            {
                if (!lua_istable(L, -1))
                {
                    softerrmsg("in return from onRequest() - value of \"head\""
                              "must be a table!");
                }
                else
                {
                    lua_pushnil(L);
                    while (lua_next(L, -2))
                    {
                        push_pair_list(&servlet->pair_list_alloc, &head,
                                lua_tostring(L, -2),
                                lua_tostring(L, -1));

                        lua_pop(L, 1);
                    }
                }
            }
            else if (strcmp(k, "foot") == 0)
            {
                if (!lua_istable(L, -1))
                {
                    softerrmsg("in return from onRequest() - value of \"foot\""
                              " must be a table!");
                }
                else
                {
                    lua_pushnil(L);
                    while (lua_next(L, -2))
                    {
                        push_pair_list(&servlet->pair_list_alloc, &foot,
                                lua_tostring(L, -2),
                                lua_tostring(L, -1));

                        lua_pop(L, 1);
                    }
                }
            }

            lua_pop(L, 1);
        }

        resp = MHD_create_response_from_buffer(
                answerlen, (void *)answerbody, MHD_RESPMEM_MUST_COPY);

        while (head)
        {
            pop_pair_list(&servlet->pair_list_alloc, &head, &k, &v);
            fprintf(stderr, "header : k = %s , v = %s\n", k, v);
            MHD_add_response_header(resp, k, v);
        }
        
        while (foot)
        {
            pop_pair_list(&servlet->pair_list_alloc, &foot, &k, &v);
            MHD_add_response_footer(resp, k, v);
        }

        ret = MHD_queue_response(connection, answercode, resp);
        MHD_destroy_response(resp);
    }
    lua_gc(L, LUA_GCRESTART, 0);
    return ret;
}

static void request_closed(void *cls, struct MHD_Connection *connection,
        void **con_cls, enum MHD_RequestTerminationCode toe)
{

    servlet_t *servlet = (servlet_t *)cls;
    conn_t *conn = (conn_t *)*con_cls;

    /* If this is a post request, we destroy the POST data parser. */
    if (conn->method_id == FH1_POST)
        MHD_destroy_post_processor(conn->postproc);

    /* free the connection struct */
    GRIM_PODALLOCFREE(conn_t, &servlet->conn_alloc, conn);

    /* NULL it out. */
    *con_cls = NULL;
}

static void requires(lua_State *L, const char *name, lua_CFunction openf)
{
#if LUA_VERSION_NUM >= 502
    luaL_requiref(L, name, openf, 1);
#else
    lua_pushcfunction(L, openf);
    lua_pushstring(L, name);
    lua_call(L, 1, 0);
#endif
}

static void softerrmsg(const char *msg)
{
    fprintf(stderr, "\n -- ERROR : %s --\n", msg);
}

static void errmsg(lua_State *L, const char *msg)
{
    fprintf(stderr, "\n -- FATAL ERROR --\n  %s: %s\n\n",
            msg, lua_tostring(L, -1));
}

static void bail(lua_State *L, const char *msg)
{
    errmsg(L, msg);
    /* TODO: do the appropriate cleanup on globals. */
    exit(1);
}

static void startup(const char *filename, lua_State **pL)
{
    lua_State *L = luaL_newstate();     /* create lua state variable */
    fprintf(stderr, "%s", " -- starting up --\n");
    luaL_openlibs(L);                   /* load lua libraries
                                         * TODO: disable file io stuff
                                         * TODO: make io.write write to log */

    /* load up our server tools */
    requires(L, "postgres", luaopen_luasql_postgres);
    requires(L, "cjson",    luaopen_cjson_safe);

    /* load the script */
    fprintf(stderr, "%s%s%s", " -- loading \"", filename, "\" --\n");
    if (luaL_loadfile(L, filename))
        bail(L, "lual_loadfile() failed");

    /* priming run on the script */
    fprintf(stderr, "%s%s%s", " -- exec \"", filename, "\" --\n");
    if (lua_pcall(L, 0, 0, 0))
        bail(L, "lua_pcall() failed");

    /* return by argument */
    *pL = L;
    fprintf(stderr, "%s", " -- running --\n");
}

static void reset(const char *filename, lua_State **pL)
{
    fprintf(stderr, "%s", " -* RESET *-\n");

    lua_close(*pL);
    startup(filename, pL);
}

static void fatalreset(const char *filename, lua_State **pL, const char *msg)
{
    errmsg(*pL, msg);
    fprintf(stderr, "%s", " -* PANIC, TRYING RESET! *-\n");

    reset(filename, pL);
}

static void init_servlet(servlet_t *servlet, const char *filename)
{
    /* TODO: add a way to tune these on target platform */
    GRIM_PODALLOCINIT(conn_t, &servlet->conn_alloc, 1024);
    GRIM_PODALLOCINIT(pair_list_t, &servlet->pair_list_alloc, 2048);

    servlet->filename = filename;
    if (stat(filename, &servlet->st) < 0)
    {
        fprintf(stderr, "%s%s%s", "FATAL: cannot stat \"", filename, "\",\n"); 
        perror(0);
        exit(1); /* TODO: clean these out and have a single exit point */
    }

    if (!S_ISREG(servlet->st.st_mode))
    {
        fprintf(stderr, "%s",
                "FATAL: servlet script must be a regular file.\n");
        exit(1); /* TODO: clean these out and have a single exit point */
    }

    startup(filename, &servlet->L);
    servlet->nconn = 0;
}

static void cleanup_servlet(servlet_t *servlet)
{
    lua_close(servlet->L);
    GRIM_PODALLOCCLEAN(conn_t,      &servlet->conn_alloc);
    GRIM_PODALLOCCLEAN(pair_list_t, &servlet->pair_list_alloc);
    memset(servlet, 0, sizeof(servlet_t));
}

static const char *parseargs(int argc, char *argv[], uint16_t *port_no)
{
    if (argc < 2 || argc > 3)
    {
        printf(USAGE, argv[0], DEFAULTPORT);
        exit(1);
    }

    *port_no = (argc > 2) ? atoi(argv[2]) : DEFAULTPORT;
    return argv[1];
}

int main(int argc, char *argv[])
{
    uint16_t port_no = 0;
    const char *filename = parseargs(argc, argv, &port_no);

    init_servlet(&servlet, filename);

    d = MHD_start_daemon(
            MHD_USE_SELECT_INTERNALLY, port_no, NULL, NULL,
            request_opened, (void *)&servlet,
            MHD_OPTION_NOTIFY_COMPLETED,
            request_closed, (void *)&servlet,
            MHD_OPTION_END);

    getchar();

    MHD_stop_daemon(d);
    cleanup_servlet(&servlet);

    return 0;
}

