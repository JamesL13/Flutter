/*
 * Flutter
 * Lightweight LUA powered HTTP server
 *
 * Copyright (C) 2015 William Whitacre
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE. */

#ifndef FLUTTER_H
#define FLUTTER_H

#include "flutterconfig.h"

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

static const char *IDENTITY =
"Flutter\nLightweight LUA powered HTTP server\nv"
FLUTTER_VERSION_STRING "\n";

static const char *MITLICENSE =
"Copyright (C) 2015 William Whitacre\n\n"
"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
"of this software and associated documentation files (the \"Software\"), to\n"
"deal in the Software without restriction, including without limitation the\n"
"rights to use, copy, modify, merge, publish, distribute, sublicense, and/or\n"
"sell copies of the Software, and to permit persons to whom the Software is\n"
"furnished to do so, subject to the following conditions:\n\n"
"The above copyright notice and this permission notice shall be included in\n"
"all copies or substantial portions of the Software.\n\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
"FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS\n"
"IN THE SOFTWARE.\n";

/* usage string */
static const char *USAGE =
    "USAGE: \n"
    "   %s LUAFILE [PORT_NO]\n\n"
    "   default port is %i\n";

/* parse command line arguments */
static const char *parseargs(int argc, char *argv[], uint16_t *port_no);

#endif

