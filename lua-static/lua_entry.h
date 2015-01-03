#ifndef LUA_ENTRY_H
#define LUA_ENTRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

int luaopen_cjson(lua_State *l);
int luaopen_cjson_safe(lua_State *l);

int luaopen_luasql_postgres(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif

