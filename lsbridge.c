#define LUA_LIB
#include "lsbridge.h"
#include "lbind/lbind.h"

LBLIB_API lbind_Type lbT_Node;
LBLIB_API lbind_Type lbT_EventSignal;
LBLIB_API lbind_Type lbT_AttrHolder;
LBLIB_API lbind_Enum lbE_EventNames;


/* node exported functions */

static int Lnode_type(lua_State *L) {
    ls_Node *node = (ls_Node*)lbind_check(L, 1, &lbT_Node);
    lua_pushinteger(L, ls_type(node));
    return 1;
}

#define GETTER_FUNC(name) \
    static int Lnode_##name(lua_State *L) { \
        ls_Node *node = (ls_Node*)lbind_check(L, 1, &lbT_Node); \
        return lbind_retrieve(L, ls_##name(node)); \
    }

GETTER_FUNC(parent)
GETTER_FUNC(prevsibling)
GETTER_FUNC(nextsibling)
GETTER_FUNC(firstchild)
GETTER_FUNC(lastchild)
GETTER_FUNC(root)
GETTER_FUNC(firstleaf)
GETTER_FUNC(lastleaf)
GETTER_FUNC(prevleaf)
GETTER_FUNC(nextleaf)

#undef GETTER_FUNC

static int Lnode_append(lua_State *L) {
    ls_Node *node = (ls_Node*)lbind_check(L, 1, &lbT_Node);
    int i, top = lua_gettop(L);
    for (i = 2; i <= top; ++i) {
        ls_Node *newnode = (ls_Node*)lbind_check(L, i, &lbT_Node);
        ls_append(node, newnode);
    }
    lua_settop(L, 1);
    return 1;
}

static int Lnode_insert(lua_State *L) {
    ls_Node *node = (ls_Node*)lbind_check(L, 1, &lbT_Node);
    int i, top = lua_gettop(L);
    for (i = 2; i <= top; ++i) {
        ls_Node *newnode = (ls_Node*)lbind_check(L, i, &lbT_Node);
        ls_insert(node, newnode);
    }
    lua_settop(L, 1);
    return 1;
}

static int Lnode_setchildren(lua_State *L) {
    ls_Node *node = (ls_Node*)lbind_check(L, 1, &lbT_Node);
    ls_Node *children = (ls_Node*)lbind_check(L, 2, &lbT_Node);
    ls_setchildren(node, children);
    lua_settop(L, 1);
    return 1;
}

static int Lnode_removeself(lua_State *L) {
    ls_Node *node = (ls_Node*)lbind_check(L, 1, &lbT_Node);
    ls_removeself(node);
    lua_settop(L, 1);
    return 1;
}

static int Lnode___len(lua_State *L) { return 0; }
static int Lnode___ipairs(lua_State *L) { return 0; }
static int Lnode___pairs(lua_State *L) { return 0; }


/* event exported functions */
 
typedef struct {
    lua_State *L;
    int nargs;
    int top;
    int traceback;
} Levent_Ctx;

static void event_handler(ls_EventSlot *slot, void *evtdata) {
    Levent_Ctx *ctx = (Levent_Ctx*)evtdata;
    lua_State *L = ctx->L;
    /* stack: args return_status(nil or string) */
    luaL_checkstack(L, ctx->nargs+2, "too many arguments");
    if (lbind_retrieve(L, slot->signal) /* 1 */
            && lbind_retrieve(L, slot)) { /* 2 */
        int i;
        lua_gettable(L, -2); /* 2->2 */
        for (i = 0; i < ctx->nargs; ++i)
            lua_pushvalue(L, -ctx->nargs-3);
        if (lua_pcall(L, ctx->nargs, 0, ctx->traceback) != LUA_OK) { /* 2->2 */
            if (lua_isnil(L, -3)) /* status */
                lua_insert(L, -3);
            else {
                lua_pushstring(L, "\n- - -\n"); /* 3 */
                lua_replace(L, -3); /* (1) */
                lua_concat(L, 3);
            }
        }
    }
    lua_settop(L, ctx->top);
}

int lsL_event_emit(lua_State *L, ls_EventSignal *signal, int evtid, int args) {
    int res = 0;
    Levent_Ctx ctx = { L, args, lua_gettop(L) + 2 };
    luaL_checkstack(L, 2, "no enough space for return value");
    /* get debug.traceback */
    lua_getglobal(L, LUA_DBLIBNAME);
    lua_getfield(L, -1, "traceback");
    lua_remove(L, -2);
    ctx.traceback = lua_isnil(L, -1) ? 0 : ctx.top - args - 2;
    lua_insert(L, ctx.traceback);
    /* set initial return status value */
    lua_pushnil(L);
    /* stack: traceback args status */
    /* emit */
    ls_event_emit(signal, evtid, (void*)&ctx);
    if (lua_isnil(L, -1))
        lua_pop(L, 1);
    else {
        lua_insert(L, -args-2); /* insert status before traceback */
        res = LUA_ERRRUN;
    }
    lua_pop(L, args + 1); /* remove args and traceback */
    return res;
}

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
    if (signal) Levent_reset(L);
    lua_pushnil(L);
    lua_setuservalue(L, 1);
    return 0;
}

typedef struct {
    ls_EventSlot slot;
    lua_State *L;
} Levent_Slot;

#define CHECK   1
#define ADDNEW  0

static int get_evtname(lua_State *L, int check, int *pidx) {
    int evtid;
    if (lua_type(L, 2) != LUA_TSTRING) {
        luaL_checkany(L, 2);
        *pidx = 2;
        return 1;
    }
    if (check)
        evtid = lbind_checkenum(L, 2, &lbE_EventNames);
    else if ((evtid = lbind_testenum(L, 2, &lbE_EventNames)) == -1)
        evtid = lbind_addenum(L, 2, &lbE_EventNames);
    luaL_checkany(L, 3);
    *pidx = 3;
    return evtid;
}

static int Levent_connect(lua_State *L) {
    ls_EventSignal *signal = lbind_check(L, 1, &lbT_EventSignal);
    Levent_Slot *slot;
    int i, idx, evtid, top = lua_gettop(L);
    evtid = get_evtname(L, ADDNEW, &idx);
    for (i = idx; i <= top; ++i) {
        slot = (Levent_Slot*)lbind_raw(L, sizeof(Levent_Slot));
        ls_event_initslot(&slot->slot, evtid, event_handler);
        ls_event_connect(signal, &slot->slot);
        slot->L = L;
        lua_pushvalue(L, i);
        lua_settable(L, 1);
    }
    return top - idx + 1;
}

static int Levent_disconnect(lua_State *L) {
    int idx, evtid = get_evtname(L, CHECK, &idx);
    int i, top = lua_gettop(L);
    lbind_check(L, CHECK, &lbT_EventSignal);
    lua_getuservalue(L, 1); /* 1 */
    if (!lua_isnil(L, -1)) {
        lua_pushnil(L); /* 2 */
        while (idx <= top && lua_next(L, -2)) { /* 2->2,3 */
            for (i = idx; i <= top && !lua_rawequal(L, i, -1); ++i)
                ;
            if (i <= top) {
                Levent_Slot *slot = (Levent_Slot*)lbind_object(L, -2);
                if (slot && slot->slot.eventid == evtid) {
                    ls_event_disconnect(&slot->slot);
                    lua_pushvalue(L, -2); /* 2->4 */
                    lua_pushnil(L); /* 5 */
                    lua_rawset(L, -5); /* 4,5->1 */
                    lua_remove(L, i);
                    --top;
                }
            }
            lua_pop(L, 1); /* (3) */
        }
    }
    lua_settop(L, 1);
    return 1;
}

static int Levent_emit(lua_State *L) {
    ls_EventSignal *signal = lbind_check(L, 1, &lbT_EventSignal);
    int idx, evtid = get_evtname(L, CHECK, &idx), top;
    top = lua_gettop(L);
    if (lsL_event_emit(L, signal, evtid, top - idx + 1) != LUA_OK)
        luaL_error(L, "error occur in emit:\n%s\n- - -", lua_tostring(L, -1));
    return 0;
}

/* attrs exported functions */

lbind_EnumItem event_names[] = {
    { "default", 1 },
    { NULL, 0 },
};

LUALIB_API int luaopen_node(lua_State *L) {
    luaL_Reg libs_getter[] = {
#define ENTRY(n) { #n, Lnode_##n }
        ENTRY(type),
        ENTRY(parent),
        ENTRY(prevsibling),
        ENTRY(nextsibling),
        ENTRY(firstchild),
        ENTRY(lastchild),
        ENTRY(root),
        ENTRY(firstleaf),
        ENTRY(lastleaf),
        ENTRY(prevleaf),
        ENTRY(nextleaf),
        { NULL, NULL },
    }, libs[] = {
        ENTRY(append),
        ENTRY(insert),
        ENTRY(setchildren),
        ENTRY(removeself),
        { NULL, NULL }
    }, libs_meta[] = {
        ENTRY(__ipairs),
        ENTRY(__len),
        ENTRY(__pairs),
#undef  ENTRY
        { NULL, NULL }
    };
    lbind_newclass_meta(L, "node", libs, NULL, &lbT_Node);
    lbind_setaccessor(L, libs_getter, NULL, &lbT_Node);
    return 1;
}

LUALIB_API int luaopen_event(lua_State *L) {
    luaL_Reg libs[] = {
#define ENTRY(n) { #n, Levent_##n }
        ENTRY(new),
        ENTRY(delete),
        ENTRY(reset),
        ENTRY(connect),
        ENTRY(disconnect),
        ENTRY(emit),
        { NULL, NULL }
    }, libs_meta[] = {
        { "__call", Levent_emit },
#undef  ENTRY
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

/* cc: flags+='-s -O2 -mdll -DLUA_BUILD_AS_DLL'
 * cc: libs+='d:/lua52/lua52.dll' run='lua test.lua'
 * cc: input+='lsnode.c lsevent.c lbind/lbind.c' output='event.dll'
 */
