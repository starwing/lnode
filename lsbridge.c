#define LUA_LIB
#include "lsbridge.h"
#include "lbind/lbind.h"

LBLIB_API lbind_Type lbT_Node;
LBLIB_API lbind_Type lbT_EventSignal;
LBLIB_API lbind_Type lbT_AttrHolder;
LBLIB_API lbind_Enum lbE_EventNames;


/* node exported functions */

/* event exported functions */

static int Levent_new(lua_State *L) {
    ls_EventSignal *signal = lbind_new(L, sizeof(ls_EventSignal), &lbT_EventSignal);
    ls_event_initsignal(signal);
    return 1;
}

static int Levent_reset(lua_State *L) {
    ls_EventSignal *signal = lbind_check(L, 1, &lbT_EventSignal);
    ls_event_reset(signal);
    lua_getuservalue(L, 1); /* 1 */
    if (!lua_isnil(L, -1)) {
        lua_pushnil(L); /* 2 */
        while (lua_next(L, -2)) { /* 2->2,3 */
            lua_pushvalue(L, -2); /* 4 */
            lua_pushnil(L); /* 5 */
            lua_rawset(L, -5); /* 4,5->1 */
            lua_pop(L, 1); /* (3) */
        }
    }
    lua_settop(L, 1);
    return 1;
}

static int Levent_delete(lua_State *L) {
    ls_EventSignal *signal = (ls_EventSignal*)lbind_test(L, 1, &lbT_EventSignal);
    printf("delete\n");
    if (signal) Levent_reset(L);
    lua_pushnil(L);
    lua_setuservalue(L, 1);
    return 0;
}

typedef struct {
    ls_EventSlot slot;
    lua_State *L;
} Levent_Slot;

typedef struct {
    lua_State *L;
    int nargs;
} Levent_Ctx;

static int name2evtid(lua_State *L, int idx) {
    int evtid;
    if ((evtid = lbind_testenum(L, idx, &lbE_EventNames)) == -1)
        evtid = lbind_addenum(L, idx, &lbE_EventNames);
    return evtid;
}

static void event_handler(ls_EventSlot *slot, void *evtdata) {
    Levent_Ctx *ctx = (Levent_Ctx*)evtdata;
    lua_State *L = ctx->L;
    if (lbind_retrieve(L, slot->signal)
            && lbind_retrieve(L, slot)) {
        int i;
        lua_gettable(L, -2);
        for (i = 0; i < ctx->nargs; ++i)
            lua_pushvalue(L, i+3);
        lua_call(L, ctx->nargs, 0);
    }
}

static void default_evtname(lua_State *L) {
    if (lua_type(L, 2) != LUA_TSTRING) {
        luaL_checkany(L, 2);
        lbind_pushenum(L, "default", &lbE_EventNames);
        lua_insert(L, 2);
    }
    luaL_checkany(L, 3);
}

static int Levent_connect(lua_State *L) {
    ls_EventSignal *signal = lbind_check(L, 1, &lbT_EventSignal);
    Levent_Slot *slot;
    default_evtname(L);
    slot = (Levent_Slot*)lbind_raw(L, sizeof(Levent_Slot));
    ls_event_initslot(&slot->slot, name2evtid(L, 2), event_handler);
    ls_event_connect(signal, &slot->slot);
    slot->L = L;
    lua_pushvalue(L, 3);
    lua_settable(L, 1);
    lua_settop(L, 3);
    return 1;
}

static int Levent_disconnect(lua_State *L) {
    lbind_check(L, 1, &lbT_EventSignal);
    default_evtname(L);
    lua_getuservalue(L, 1); /* 1 */
    if (!lua_isnil(L, -1)) {
        int evtid = lbind_checkenum(L, 2, &lbE_EventNames);
        lua_pushnil(L); /* 2 */
        while (lua_next(L, -2)) { /* 2->2,3 */
            if (lua_rawequal(L, 3, -1)) {
                Levent_Slot *slot = (Levent_Slot*)lbind_object(L, -2);
                if (slot && slot->slot.eventid == evtid) {
                    ls_event_disconnect(&slot->slot);
                    lua_pop(L, 1);
                    lua_pushnil(L);
                    lua_rawset(L, -3);
                    break;
                }
            }
            lua_pop(L, 1);
        }
    }
    lua_settop(L, 1);
    return 1;
}

static int Levent_emit(lua_State *L) {
    ls_EventSignal *signal = lbind_check(L, 1, &lbT_EventSignal);
    int evtid, top;
    default_evtname(L);
    evtid = lbind_checkenum(L, 2, &lbE_EventNames);
    top = lua_gettop(L);
    Levent_Ctx ctx = { L, top - 2 };
    ls_event_emit(signal, evtid, (void*)&ctx);
    return 0;
}

/* attrs exported functions */

lbind_EnumItem event_names[] = {
    { "default", 1 },
    { NULL, 0 },
};

LUALIB_API int luaopen_node(lua_State *L);

LUALIB_API int luaopen_event(lua_State *L) {
    luaL_Reg libs[] = {
#define ENTRY(n) { #n, Levent_##n }
        ENTRY(new),
        ENTRY(delete),
        ENTRY(reset),
        ENTRY(connect),
        ENTRY(disconnect),
        ENTRY(emit),
#undef  ENTRY
        { NULL, NULL }
    }, libs_meta[] = {
        { "__call", Levent_emit },
        { NULL, NULL }
    };
    lbind_newenum(L, "event name", event_names, &lbE_EventNames);
    lbind_newclass_meta(L, "event", libs, NULL, &lbT_EventSignal);
    return 1;
}

LUALIB_API int luaopen_attrs(lua_State *L);

LUALIB_API int luaopen_bridge(lua_State *L) {
    static luaL_Reg libbridge[] = {
        { NULL, NULL }
    };

    luaL_newlib(L, libbridge);
    return 1;
}

/* cc: flags+='-ggdb -O2 -mdll -DLUA_BUILD_AS_DLL'
 * cc: libs+='d:/lua52/lua52.dll' run='lua test.lua'
 * cc: input+='lsevent.c lbind/lbind.c' output='event.dll'
 */
