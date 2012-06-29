#ifndef lsexport_h
#define lsexport_h


#include <lua.h>
#include <lauxlib.h>

#define lsL_setweak(L, mode) ( \
        lua_newtable(L), lua_pushliteral(L, mode), \
        lua_setfield(L, -2, "__mode"), lua_setmetatable(L, -2))

#define lsL_setregistry(L, n, name) \
    (lua_pushvalue((L),(n)),lua_setfield((L),LUA_REGISTRYINDEX,(name)))

#define lsL_getregistry(L, name) \
    (lua_getfield((L),LUA_REGISTRYINDEX,(name)))

#define lsL_ensuretop(L, n, errmsg) \
    ((void)(lua_gettop(L) < n && luaL_argerror(L, 1, errmsg)), lua_settop(L, n))

void **lsL_testudata_uv (lua_State *L, int narg, int uv);
void  *lsL_checkudata   (lua_State *L, int narg, void **p);
int    lsL_initudata_uv (lua_State *L, int uv_pbox, int uv_mt);
int    lsL_tostring     (lua_State *L, void **p, const char *tname);


#ifdef LS_EXPORT_NODE
#include "lsnode.h"
#define LS_NODE_PBOX "lsnode pbox entry"
#define LS_NODE_TYPE "Node"

#define lsL_testnode(L,n) ((ls_Node**)luaL_testudata((L), (n), LS_EVENT_TYPE))
#define lsL_checknode(L,n) (*(ls_Node**)lsL_checkudata((L), (n),\
            (void**)lsL_testnode((L),(n))))
#endif /* LS_EXPORT_NODE */


#ifdef LS_EXPORT_EVENT
#include "lsevent.h"
#define LS_EVENT_PBOX   "lsevent pbox entry"
#define LS_EVENT_CBOX   "lsevent callback box"
#define LS_EVENT_SIGN_TYPE   "EventSignal"
#define LS_EVENT_SLOT_TYPE   "EventSlot"

#define lsL_testsignal(L,n) ((ls_EventSignal**)luaL_testudata((L), (n), LS_EVENT_SIGN_TYPE))
#define lsL_checksignal(L,n) (*(ls_EventSignal**)lsL_checkudata((L), (n),\
            (void**)lsL_testsignal((L),(n))))
#define lsL_testslot(L,n) ((ls_EventSlot**)luaL_testudata((L), (n), LS_EVENT_SLOT_TYPE))
#define lsL_checkslot(L,n) (*(ls_EventSlot**)lsL_checkudata((L), (n),\
            (void**)lsL_testslot((L),(n))))
#endif /* LS_EXPORT_EVENT */


#endif /* lsexport_h */
