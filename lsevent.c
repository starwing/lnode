#include "lsevent.h"

#include <stddef.h>

void ls_event_initsignal(ls_EventSignal *signal) {
    signal->slots = NULL;
}

void ls_event_initslot(ls_EventSlot *slot, int eventid, ls_EventHandler f, void *ud) {
    slot->prev = slot->next = slot;
    slot->eventid = eventid;
    slot->f = f;
    slot->ud = ud;
    slot->signal = NULL;
}

void ls_event_connect(ls_EventSignal *signal, ls_EventSlot *slot) {
    if (slot->signal != NULL)
        ls_event_disconnect(slot);
    if (signal->slots == NULL)
        signal->slots = slot;
    else {
        ls_EventSlot *head = signal->slots;
        head->prev->next = slot;
        slot->prev = head->prev;
        slot->next = head;
        head->prev = slot;
    }
    slot->signal = signal;
}

void ls_event_disconnect(ls_EventSlot *slot) {
    int isempty = (slot == slot->next);
    ls_EventSignal *signal = slot->signal;
    if (signal) {
        if (signal->slots == slot)
            signal->slots = isempty ? NULL : slot->next;
        if (!isempty) {
            slot->prev->next = slot->next;
            slot->next->prev = slot->prev;
            slot->prev = slot->next = slot;
        }
        slot->signal = NULL;
    }
}

void ls_event_emit(ls_EventSignal *signal, int eventid, void *evtdata) {
    ls_EventSlot *slot, *next, *firstmatch = NULL;
    if ((slot = signal->slots)) {
        do {
            next = slot->next;
            if (slot->eventid == eventid) {
                if (firstmatch == NULL)
                    firstmatch = slot;
                slot->f(slot->ud, evtdata, signal, slot);
            }
        } while ((slot = next) != signal->slots);
        signal->slots = firstmatch;
    }
}

#ifdef LS_EXPORT_EVENT
#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

#include "lsexport.h"

#define UV_CB   1
#define UV_UD   2
#define UV_SIGN 3
#define UV_SELF 4

static int Lcb_impl(lua_State *L) {
#define optuvidx(n,sn) (!lua_isnoneornil((L),(sn))?(sn):lua_upvalueindex(n))
    lua_pushvalue(L, lua_upvalueindex(UV_CB));
    lua_pushvalue(L, optuvidx(2, UV_UD));
    lua_pushvalue(L, optuvidx(3, UV_SIGN));
    lua_pushvalue(L, optuvidx(4, UV_SELF));
#undef optuvidx
    if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
        lua_getglobal(L, "panic");
        lua_pushvalue(L, -2);
        lua_pcall(L, 1, 0, 0);
    }
    return 0;
}

static void Lcb_helper(void *ud, void *evtdata, ls_EventSignal *signal, ls_EventSlot *self) {
    lua_State *L = (lua_State*)ud;
    int top = lua_gettop(L);
    (void)signal; /* unused argument */
    lsL_getregistry(L, LS_EVENT_CBOX);
    lua_rawgetp(L, -1, (void*)self);
    if (!lua_isnil(L, -1)) {
        if (evtdata != NULL) {
            lsL_getregistry(L, LS_EVENT_PBOX);
            lua_rawgetp(L, -1, evtdata);
            lua_setupvalue(L, -3, UV_UD);
            lua_call(L, 0, 0);
        }
        else {
            int i, args = lua_tointeger(L, top);
            for (i = 0; i < args; ++i)
                lua_pushvalue(L, top-args+i);
            lua_call(L, args, 0);
        }
    }
    lua_settop(L, top);
}

#define UV_PBOX      1
#define UV_SIGN_META 2
#define UV_SLOT_META 3

static int Lnewsignal(lua_State *L) {
    struct ls_SignalWithPtr {
        ls_EventSignal *p;
        ls_EventSignal signal;
    } *swp = (struct ls_SignalWithPtr*)lua_newuserdata(L,
            sizeof(struct ls_SignalWithPtr));
    swp->p = &swp->signal;
    ls_event_initsignal(swp->p);
    return 1;
}

static int Lnewslot(lua_State *L) {
    int eventid = luaL_checkint(L, 1);
    struct ls_SlotWithPtr {
        ls_EventSlot *p;
        ls_EventSlot slot;
    } *swp = (struct ls_SlotWithPtr*)lua_newuserdata(L,
            sizeof(struct ls_SlotWithPtr));
    swp->p = &swp->slot;
    ls_event_initslot(swp->p, eventid, Lcb_helper, L);
    return 1;
}

static int Linitsignal(lua_State *L) {
    lua_settop(L, 2);
    return lsL_initudata_uv(L, 1, UV_SIGN_META);
}

static int Linitslot(lua_State *L) {
    lua_settop(L, 2);
    return lsL_initudata_uv(L, 1, UV_SLOT_META);
}

#define testsignal(L, n) \
    ((ls_EventSignal**)lsL_testudata_uv(L, n, UV_SIGN_META))
#define checksignal(L, n) \
    ((ls_EventSignal*)lsL_checkudata(L, n, lsL_testudata_uv(L, n, UV_SIGN_META)))

#define testslot(L, n) \
    ((ls_EventSlot**)lsL_testudata_uv(L, n, UV_SLOT_META))
#define checkslot(L, n) \
    ((ls_EventSlot*)lsL_checkudata(L, n, lsL_testudata_uv(L, n, UV_SLOT_META)))

static int Ltostring(lua_State *L) {
    ls_EventSignal **psignal = testsignal(L, 1);
    return psignal ?
        lsL_tostring(L, (void**)psignal, LS_EVENT_SIGN_TYPE) :
        lsL_tostring(L, (void**)testslot(L, 1), LS_EVENT_SLOT_TYPE);
}

static void setcbinfo(lua_State *L, ls_EventSlot *slot, int upvalue) {
    lsL_getregistry(L, LS_EVENT_CBOX); /* 1 */
    lua_rawgetp(L, -1, (void*)slot); /* 2 */
    if (lua_isnil(L, -1)) {
        lua_settop(L, lua_gettop(L) + 3); /* 2,3,4,5 */
        lua_pushcclosure(L, Lcb_impl, 4); /* 2,3,4,5->2 */
        lua_pushvalue(L, -1); /* 2->3 */
        lua_rawsetp(L, -3, (void*)slot); /* 3->1 */
    }
    lua_remove(L, -2); /* (1) */
    lua_pushvalue(L, -2);
    lua_setupvalue(L, -2, upvalue);
}

static int Ldeletesignal(lua_State *L) {
    ls_EventSignal **psignal = testsignal(L, 1);
    if (psignal && *psignal) {
        while ((*psignal)->slots)
            ls_event_disconnect((*psignal)->slots);
        lua_pushnil(L);
        lua_setmetatable(L, 1);
        *psignal = NULL;
    }
    return 0;
}

static int Ldeleteslot(lua_State *L) {
    ls_EventSlot **pslot = testslot(L, 1);
    if (pslot && *pslot) {
        ls_event_disconnect(*pslot);
        lua_pushnil(L);
        setcbinfo(L, *pslot, UV_SIGN);
        lua_pushnil(L);
        lua_setupvalue(L, -2, UV_SELF);
        lua_pushnil(L);
        lua_setmetatable(L, 1);
        *pslot = NULL;
    }
    return 0;
}

static int Lemit(lua_State *L) {
    ls_EventSignal *signal = checksignal(L, 1);
    int evtid = luaL_checkint(L, 2);
    void **ud = (void**)lua_touserdata(L, 3);
    lua_pushinteger(L, lua_gettop(L) - 2);
    ls_event_emit(signal, evtid, ud == NULL ? NULL : *ud);
    return 0;
}

static int Lcallback(lua_State *L) {
    ls_EventSlot *slot = checkslot(L, 1);
    lua_settop(L, 2);
    lua_pushvalue(L, 1);
    setcbinfo(L, slot, 4);
    lua_getupvalue(L, -1, UV_SELF);
    if (!lua_isnone(L, 2)) {
        lua_pushvalue(L, 2);
        lua_setupvalue(L, -3, UV_CB);
    }
    return 1;
}

static int Lconnect(lua_State *L) {
    ls_EventSignal *signal = checksignal(L, 1);
    ls_EventSlot *slot = checkslot(L, 2);
    ls_event_connect(signal, slot);
    lua_pushvalue(L, 1);
    setcbinfo(L, slot, UV_SIGN);
    lua_pushvalue(L, 2);
    lua_setupvalue(L, -2, UV_SELF);
    lua_settop(L, 2);
    return 1;
}

static int Ldisconnect(lua_State *L) {
    ls_EventSlot *slot = checkslot(L, 1);
    ls_event_disconnect(slot);
    lua_pushnil(L);
    setcbinfo(L, slot, UV_SIGN);
    lua_pushnil(L);
    lua_setupvalue(L, -2, UV_SELF);
    lua_settop(L, 1);
    return 1;
}

static int Linit(lua_State *L) {
    luaL_Reg evtlibs[] = {
#define ENTRY(n) { #n, L##n },
        ENTRY(newsignal)
        ENTRY(newslot)
        ENTRY(initsignal)
        ENTRY(initslot)
        ENTRY(deletesignal)
        ENTRY(deleteslot)
        ENTRY(tostring)
        ENTRY(emit)
        ENTRY(callback)
        ENTRY(connect)
        ENTRY(disconnect)
#undef  ENTRY
        { NULL, NULL }
    };
    lua_newtable(L);
    lsL_setweak(L, "k");
    lua_setfield(L, LUA_REGISTRYINDEX, LS_EVENT_CBOX);
    lsL_ensuretop(L, 3, "(pbox, signalmt, slotmt) expected");
    lsL_setregistry(L, 1, LS_EVENT_PBOX);
    lsL_setregistry(L, 2, LS_EVENT_SIGN_TYPE);
    lsL_setregistry(L, 3, LS_EVENT_SLOT_TYPE);
    lua_newtable(L);
    lua_insert(L, 1);
    luaL_setfuncs(L, evtlibs, 3);
    return 1;
}

LUALIB_API int luaopen_node_event(lua_State *L) {
    lua_pushcfunction(L, Linit);
    return 1;
}
#endif /* LS_EXPORT_EVENT */
/*
 * cc: flags+='-s -O2 -mdll -pedantic -DLUA_BUILD_AS_DLL -Id:/lua52/include'
 * cc: flags+='-DLS_EXPORT_NODE -DLS_EXPORT_ATTR -DLS_EXPORT_EVENT'
 * cc: libs+='d:/lua52/lua52.dll'
 * cc: input='*.c' output='c-node.dll'
 */
