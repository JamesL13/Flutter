
/* configuration */
#define POSTBUFFERSIZE  512
#define MAXCLIENTS      12000
#define DEFAULTPORT     8080

enum /* HTTP 1.1 methods */
{
    FH1_GET,
    FH1_PUT,
    FH1_DELETE,
    FH1_POST,
};

/* keeping track of pair lists */
typedef struct pair_list
{
    const char *k, *v;
    struct pair_list *next;
} pair_list_t;

/* User data for tracking table index when iterating over post data. */
typedef struct table_indexer
{
    lua_State *L;
    int i;
} table_indexer_t;

/* persistent connection stuff */
typedef struct conn
{
    /* Stuff for dealing with post errors. */
    struct MHD_PostProcessor *postproc;
    table_indexer_t tab;
    int posterr;

    /* Essentials */
    int method_id;
} conn_t;

/* define podalloc macros */
GRIM_PODALLOCDEF(conn_t);
GRIM_PODALLOCDEF(pair_list_t);

typedef struct servlet
{
    /* special high performance block allocators
     * cut out a lot of peanuts allocation under heavy load */
    GRIM_PODALLOCTYPE(conn_t) conn_alloc;
    GRIM_PODALLOCTYPE(pair_list_t) pair_list_alloc;

    const char *filename;
    struct stat st;

    lua_State *L;

    size_t nconn;
    FILE *logf;
} servlet_t;

/* push a pair on to the list */
inline void push_pair_list(GRIM_PODALLOCTYPE(pair_list_t) *alc,
        pair_list_t **p, const char *k, const char *v)
{
    pair_list_t *np = GRIM_PODALLOCALLOC(pair_list_t, alc);
    np->k = k;
    np->v = v;
    np->next = *p;
    *p = np;
}

/* pop a pair off the list and get the key value pointers */
inline void pop_pair_list(GRIM_PODALLOCTYPE(pair_list_t) *alc,
        pair_list_t **p, const char **k, const char **v)
{
    pair_list_t *np = (*p)->next;

    *k = (*p)->k;
    *v = (*p)->v;

    GRIM_PODALLOCFREE(pair_list_t, alc, *p);
    *p = np;
}

/* initialize table indexer */
static void init_table_indexer(lua_State *L, table_indexer_t *tab);

/* build a table of post data on cls */
static int post_iter(void *cls, enum MHD_ValueKind kind, const char *key,
              const char *filename, const char *content_type,
              const char *transfer_encoding, const char *data, uint64_t off,
              size_t size);

/* build a table from key value pairs */
static int iter_pairs(void *cls, enum MHD_ValueKind kind, 
                   const char *key, const char *value);

/* helper function for reading pairs inline with lua */
static void read_pairs(lua_State *L,
        struct MHD_Connection *conn, 
        enum MHD_ValueKind kind);

/* handle connection open */
static int request_opened(void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **con_cls);

/* handle connection closed */
static void request_closed(void *cls, struct MHD_Connection *connection,
        void **con_cls, enum MHD_RequestTerminationCode toe);

/* lua utils */
static void requires(lua_State *L, const char *name, lua_CFunction openf);
static void softerrmsg(const char *msg);
static void errmsg(lua_State *L, const char *msg);
static void bail(lua_State *L, const char *msg);
static void startup(const char *filename, lua_State **pL);
static void reset(const char *filename, lua_State **pL);
static void fatalreset(const char *filename, lua_State **pL, const char *msg);

/* init and cleanup */
static void init_servlet(servlet_t *servlet, const char *filename);
static void cleanup_servlet(servlet_t *servlet);

/* usage string */
static const char *USAGE =
    "USAGE: \n"
    "   %s LUAFILE [PORT_NO]\n\n"
    "   default PORT_NO = %i\n";

/* parse command line arguments */
static const char *parseargs(int argc, char *argv[], uint16_t *port_no);

