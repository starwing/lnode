#include "lsevent.h"

#include <stddef.h>

void ls_event_initslot(ls_EventSlot *slot) {
    slot->handlers = NULL;
}

void ls_event_inithandler(ls_EventHandler *handler, int eventid, ls_EventProc f, void *ud) {
    handler->eventid = eventid;
    handler->f = f;
    handler->ud = ud;
    handler->slot = NULL;
    handler->prev = handler->next = handler;
}

void ls_event_addhandler(ls_EventSlot *slot, ls_EventHandler *newh) {
    if (newh->slot != NULL)
        ls_event_removehandler(newh);
    if (slot->handlers == NULL)
        slot->handlers = newh;
    else {
        ls_EventHandler *handler = slot->handlers;
        handler->prev->next = newh;
        newh->prev = handler->prev;
        newh->next = handler;
        handler->prev = newh;
    }
    newh->slot = slot;
}

void ls_event_removehandler(ls_EventHandler *handler) {
    int isempty = (handler == handler->next);
    ls_EventSlot *slot = handler->slot;
    if (slot) {
        if (slot->handlers == handler)
            slot->handlers = isempty ? NULL : handler->next;
        if (!isempty) {
            handler->prev->next = handler->next;
            handler->next->prev = handler->prev;
            handler->prev = handler->next = handler;
        }
        handler->slot = NULL;
    }
}

void ls_event_emit(ls_EventSlot *slot, int eventid, void *evtdata) {
    ls_EventHandler *handler, *next, *firstmatch = NULL;
    if ((handler = slot->handlers)) {
        do {
            next = handler->next;
            if (handler->eventid == eventid) {
                if (firstmatch == NULL)
                    firstmatch = handler;
                handler->f(handler->ud, evtdata, slot, handler);
            }
        } while ((handler = next) != slot->handlers);
        slot->handlers = firstmatch;
    }
}

#ifdef LS_EXPORT_EVENT
#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

#include "lsnode.h"
#include "lsexport.h"

typedef struct {
    ls_Node node;
    ls_EventSlot slot;
} lsL_EventNode;

static int Lcb_impl(lua_State *L) {
#define optuvidx(n,sn) (!lua_isnoneornil((L),(sn))?(sn):lua_upvalueindex(n))
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_pushvalue(L, optuvidx(2, 1));
    lua_pushvalue(L, optuvidx(3, 2));
    lua_pushvalue(L, optuvidx(4, 3));
#undef optuvidx
    if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
        lua_getglobal(L, "panic");
        lua_pushvalue(L, -2);
        lua_pcall(L, 1, 0, 0);
    }
    return 0;
}

static void Lcb_helper(void *ud, void *evtdata, ls_EventSlot *slot, ls_EventHandler *self) {
    lua_State *L = (lua_State*)ud;
    int top = lua_gettop(L);
    (void)slot; /* unused argument */
    lua_getfield(L, LUA_REGISTRYINDEX, LS_EVENT_CBOX);
    lua_rawgetp(L, -1, (void*)self);
    if (!lua_isnil(L, -1)) {
        if (evtdata != NULL) {
            lua_getfield(L, LUA_REGISTRYINDEX, LS_EVENT_PBOX);
            lua_rawgetp(L, -1, evtdata);
            lua_setupvalue(L, -3, 2);
            lua_pop(L, 1);
            lua_call(L, 0, 0);
        }
        else {
            lua_pushvalue(L, top);
            lua_call(L, 1, 0);
        }
    }
    lua_settop(L, top);
}

static int Leventnode(lua_State *L) {
    int type = luaL_optint(L, 1, 0);
    struct ls_NodeWithPtr {
        ls_Node *p;
        lsL_EventNode node;
    } *nwp = (struct ls_NodeWithPtr*)lua_newuserdata(L,
            sizeof(struct ls_NodeWithPtr));
    nwp->p = &nwp->node.node;
    ls_initnode(&nwp->node.node, type);
    ls_event_initslot(&nwp->node.slot);
    return 1;
}

static int Lnew(lua_State *L) {
    int eventid = luaL_checkint(L, 1);
    struct ls_HandlerWithPtr {
        ls_EventHandler *p;
        ls_EventHandler handler;
    } *hwp = (struct ls_HandlerWithPtr*)lua_newuserdata(L,
            sizeof(struct ls_HandlerWithPtr));
    hwp->p = &hwp->handler;
    ls_event_inithandler(hwp->p, eventid, Lcb_helper, L);
    return 1;
}

static int Lemit(lua_State *L) {
    lsL_EventNode *node = (lsL_EventNode*)lsL_checknode(L, 1);
    int evtid = luaL_checkint(L, 2);
    void **ud = (void**)lua_touserdata(L, 3);
    if (lua_isnone(L, 3)) lua_settop(L, 3);
    ls_event_emit(&node->slot, evtid, ud == NULL ? NULL : *ud);
    return 0;
}

static ls_EventHandler *checkhandler(lua_State *L, int narg) {
    return (ls_EventHandler*)lsL_checkudata(L, narg, lsL_testudata_uv(L, narg));
}

static void setcbinfo(lua_State *L, ls_EventHandler *handler, int upvalue) {
    lua_getfield(L, LUA_REGISTRYINDEX, LS_EVENT_CBOX);
    lua_rawgetp(L, -1, (void*)handler);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushnil(L);
        lua_pushcclosure(L, Lcb_impl, 4);
        lua_pushvalue(L, -1);
        lua_rawsetp(L, -3, (void*)handler);
    }
    lua_pushvalue(L, -3);
    lua_setupvalue(L, -2, upvalue);
}

static int Lcallback(lua_State *L) {
    ls_EventHandler *handler = checkhandler(L, 1);
    lua_settop(L, 2);
    lua_pushvalue(L, 1);
    setcbinfo(L, handler, 4);
    lua_getupvalue(L, -1, 1);
    if (!lua_isnone(L, 2)) {
        lua_pushvalue(L, 2);
        lua_setupvalue(L, -3, 1);
    }
    return 1;
}

static int Laddeventhandler(lua_State *L) {
    lsL_EventNode *node = (lsL_EventNode*)lsL_checknode(L, 1);
    ls_EventHandler *handler = checkhandler(L, 2);
    ls_event_addhandler(&node->slot, handler);
    lua_pushvalue(L, 1);
    setcbinfo(L, handler, 3);
    lua_pushvalue(L, 2);
    lua_setupvalue(L, -2, 4);
    lua_settop(L, 2);
    return 1;
}

static int Lremoveeventhandler(lua_State *L) {
    ls_EventHandler *handler = checkhandler(L, 2);
    ls_event_removehandler(handler);
    lua_pushnil(L);
    setcbinfo(L, handler, 3);
    lua_pushnil(L);
    lua_setupvalue(L, -2, 4);
    lua_settop(L, 1);
    return 1;
}

static int Linit(lua_State *L) {
    luaL_Reg evtlib[] = {
#define ENTRY(n) { #n, L##n },
        ENTRY(eventnode)
        ENTRY(new)
        ENTRY(emit)
        ENTRY(callback)
        ENTRY(addeventhandler)
        ENTRY(removeeventhandler)
#undef  ENTRY
        { NULL, NULL }
    };
    lua_newtable(L);
    lua_newtable(L);
    lua_pushliteral(L, "k");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, LS_EVENT_CBOX);
    return LS_INIT_MODULE(EVENT, evtlib);
}

LUALIB_API int luaopen_node_event(lua_State *L) {
    lua_pushcfunction(L, Linit);
    return 1;
}
#endif /* LS_EXPORT_EVENT */
/*
 * cc: flags+='-ggdb -O2 -mdll -pedantic -DLUA_BUILD_AS_DLL -Id:/lua52/include'
 * cc: flags+='-DLS_EXPORT_NODE -DLS_EXPORT_ATTR -DLS_EXPORT_EVENT'
 * cc: libs+='d:/lua52/lua52.dll'
 * cc: input='*.c' output='c-node.dll'
 */
