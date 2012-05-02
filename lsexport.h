#ifndef lsexport_h
#define lsexport_h


#include <lua.h>
#include <lauxlib.h>

#define LS_INIT_MODULE(NAME, l) \
  ( (void)(lua_gettop(L) < 2 && luaL_argerror(L, 1, "(pbox, metatable) expected")),  \
    lua_newtable(L),  lua_pushvalue(L, 1), lua_pushvalue(L, 2), luaL_setfuncs(L, l, 2), \
    lua_pushvalue(L, 1), lua_setfield(L, LUA_REGISTRYINDEX, LS_##NAME##_PBOX), \
    lua_pushvalue(L, 2), lua_setfield(L, LUA_REGISTRYINDEX, LS_##NAME##_TYPE), 2 ) \

#ifdef LS_EXPORT_NODE
#include "lsnode.h"
#define LS_NODE_PBOX "lsnode pbox entry"
#define LS_NODE_TYPE "Node"

void **lsL_testudata_uv (lua_State *L, int narg);
void  *lsL_checkudata   (lua_State *L, int narg, void **p);

void lsL_getnodeud (lua_State *L, ls_Node *node);

#define lsL_testnode(L,n) ((ls_Node**)luaL_testudata((L), (n), LS_EVENT_TYPE))
#define lsL_checknode(L,n) (*(ls_Node**)lsL_checkudata((L), (n),\
            (void**)lsL_testnode((L),(n))))
#endif /* LS_EXPORT_NODE */

#ifdef LS_EXPORT_EVENT
#include "lsevent.h"
#define LS_EVENT_PBOX   "lsevent pbox entry"
#define LS_EVENT_CBOX   "lsevent callback box"
#define LS_EVENT_TYPE   "EventHandler"

#define lsL_testhandler(L,n) ((ls_Node**)luaL_testudata((L), (n), LS_EVENT_TYPE))
#define lsL_checkhandler(L,n) (*(ls_Node**)lsL_checkudata((L), (n),\
            (void**)lsL_testhandler((L),(n))))
#endif /* LS_EXPORT_EVENT */


#endif /* lsexport_h */
