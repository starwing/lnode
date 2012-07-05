#define LUA_LIB
#include "lsbridge.h"
#include "lbind/lbind.h"


#include <assert.h>


LBLIB_API lbind_Type lbT_Node;
LBLIB_API lbind_Type lbT_EventSignal;
LBLIB_API lbind_Type lbT_AttrHolder;
LBLIB_API lbind_Enum lbE_EventNames;


/* node exported functions */

struct ls_LuaNode {
    ls_Node base;
    int n;
};

ls_LuaNode *lsL_node_new(lua_State *L, int type, size_t nodesize) {
    ls_LuaNode *node = lbind_new(L, sizeof(ls_LuaNode) + sizeof(nodesize), &lbT_Node);
    ls_initnode(&node->base, type);
    node->n = 0;
    return node + 1;
}

ls_Node *lsL_node(ls_LuaNode *node) {
    return &(node - 1)->base;
}

void lsL_node_setuservalue(lua_State *L, int idx) {
    /* stack: uservalue */
    ls_LuaNode *node = (ls_LuaNode*)lbind_test(L, idx, &lbT_Node);
    int pos = 1;
    if (idx < 0) idx += lua_gettop(L) + 1;
#ifndef NDEBUG
    assert(node);
    lua_getuservalue(L, idx);
    assert(lua_isnil(L, -1));
    lua_pop(L, 1);
#endif
    for (;; ++pos) {
        ls_Node *newnode;
        lua_rawgeti(L, -1, pos); /* uservalue->1 */
        if (lua_isnil(L, -1)) { lua_pop(L, 1); break; }
        newnode = (ls_Node*)lbind_test(L, -1, &lbT_Node);
        if (newnode == NULL)
            luaL_error(L, "node expected in table index %d, got %s",
                    pos, luaL_typename(L, -1));
        ls_appendchild(&node->base, newnode);
        ++node->n;
        lua_pushvalue(L, idx); /* 2 */
        lua_setfield(L, -2, "parent"); /* 2->1 */
        lua_pop(L, 1); /* (1) */
    }
    lua_pushvalue(L, -1); /* uservalue->1 */
    lua_setuservalue(L, idx); /* (1) */
    lua_rawgeti(L, -1, pos);
    while (lua_next(L, -2)) {
        if (lua_type(L, -2) == LUA_TSTRING) {
            lua_pushvalue(L, -2);
            lua_insert(L, -2);
            lua_settable(L, idx);
        }
        else
            lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

static int Lnode_new(lua_State *L) {
    lsL_node_new(L, 0, 0);
    if (lua_istable(L, 1)) {
        lua_pushvalue(L, 1);
        lsL_node_setuservalue(L, -2);
    }
    return 1;
}

static int Lnode_delete(lua_State *L) {
    return 0;
}

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

static void tinsert(lua_State *L, int pos, int count, int n) {
    int i;
    for (i = n; i >= pos; --i) {
        lua_rawgeti(L, -1, i);
        lua_rawseti(L, -2, i+count);
    }
}

static void tremove(lua_State *L, int pos, int count, int n) {
    int i;
    for (i = pos; i <= n; ++i) {
        lua_rawgeti(L, -1, i+count);
        lua_rawseti(L, -2, i);
    }
}

static int get_pos(lua_State *L, int def_pos, int *pidx, int n) {
    int pos;
    *pidx = 3;
    if ((pos = lua_tonumber(L, 2)) == 0 && !lua_isnumber(L, 2))
        pos = def_pos, *pidx = 2;
    if (pos < 0 && -pos <= n)
        pos += n + 1;
    if (def_pos == 1 ? pos < 1 || pos > n + 1
                     : pos < 0 || pos > n) {
        lua_pushfstring(L, "node index %d out of bound 1~%d", pos, n);
        return luaL_argerror(L, 2, lua_tostring(L, -1));
    }
    return pos;
}

static int Lnode_append(lua_State *L) {
    ls_LuaNode *node = (ls_LuaNode*)lbind_check(L, 1, &lbT_Node);
    ls_Node *child;
    int i, top = lua_gettop(L);
    int pos = get_pos(L, node->n, &i, node->n);
    lua_getuservalue(L, 1);
    if (ls_firstchild(&node->base) == NULL)
        child = NULL;
    else {
        lua_rawgeti(L, -1, pos);
        child = (ls_Node*)lbind_object(L, -1);
        tinsert(L, pos+1, top - i + 1, node->n);
    }
    for (; i <= top; ++i, ++node->n) {
        ls_Node *newnode = (ls_Node*)lbind_check(L, i, &lbT_Node);
        if (child == NULL)
            ls_appendchild(&node->base, newnode);
        else
            ls_append(child, newnode);
        child = newnode;
        lua_pushvalue(L, 1);
        lua_setfield(L, i, "parent");
        lua_pushvalue(L, i);
        lua_rawseti(L, -2, ++pos);
    }
    lua_settop(L, 1);
    return 1;
}

static int Lnode_insert(lua_State *L) {
    ls_LuaNode *node = (ls_LuaNode*)lbind_check(L, 1, &lbT_Node);
    ls_Node *child;
    int i, top = lua_gettop(L);
    int pos = get_pos(L, 1, &i, node->n);
    lua_getuservalue(L, 1);
    if (ls_firstchild(&node->base) == NULL)
        child = NULL;
    else {
        lua_rawgeti(L, -1, pos);
        child = (ls_Node*)lbind_object(L, -1);
        tinsert(L, pos, top - i + 1, node->n);
    }
    for (; i <= top; ++i, ++node->n) {
        ls_Node *newnode = (ls_Node*)lbind_check(L, i, &lbT_Node);
        if (child == NULL)
            ls_insertchild(&node->base, child = newnode);
        else
            ls_insert(child, newnode);
        lua_pushvalue(L, 1);
        lua_setfield(L, i, "parent");
        lua_pushvalue(L, i);
        lua_rawseti(L, -2, ++pos);
    }
    lua_settop(L, 1);
    return 1;
}

static int Lnode_remove(lua_State *L) {
    ls_LuaNode *node = (ls_LuaNode*)lbind_check(L, 1, &lbT_Node);
    ls_Node *child;
    int i, pos = get_pos(L, node->n, &i, node->n);
    int count = luaL_optint(L, 3, 1);
    lua_getuservalue(L, 1);
    lua_rawgeti(L, -1, pos);
    child = (ls_Node*)lbind_object(L, -1);
    if (child) {
        for (i = 0; i < count; ++i) {
            ls_Node *next = ls_nextsibling(child);
            ls_removeself(child);
            child = next;
            lua_rawgeti(L, -2, pos+i);
            lua_pushnil(L);
            lua_setfield(L, -2, "parent");
            lua_pop(L, 1);
        }
        tremove(L, pos, count, node->n);
        node->n -= count;
    }
    lua_settop(L, 1);
    return 1;
}

static int Lnode_removeself(lua_State *L) {
    ls_Node *node = (ls_Node*)lbind_check(L, 1, &lbT_Node);
    ls_removeself(node);
    lua_settop(L, 1);
    return 1;
}

static int Lnode___newindex(lua_State *L) {
    ls_LuaNode *node = (ls_LuaNode*)lbind_check(L, 1, &lbT_Node);
    int idx;
    if ((idx = lua_tonumber(L, 0)) != 0 || lua_isnumber(L, 1)) {
        ls_Node *newnode = (ls_Node*)lbind_check(L, 3, &lbT_Node);
        if (idx < 0 && -idx <= node->n)
            idx += node->n + 1;
        if (idx < 0 || idx > node->n + 1)
            return luaL_error(L, "index %d out of bound 1~%d", idx, node->n);
        lua_getuservalue(L, 1);
        if (idx == node->n + 1)
            ls_appendchild(&node->base, newnode);
        else {
            ls_Node *prev;
            lua_rawgeti(L, -1, idx);
            prev = (ls_Node*)lbind_object(L, -1);
            if (prev) ls_append(&node->base, newnode);
            lua_pushnil(L);
            lua_setfield(L, -2, "parent");
        }
        lua_pushvalue(L, 1);
        lua_setfield(L, 3, "parent");
    }
    else {
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_insert(L, 1);
        lua_call(L, 3, 1);
    }
    return 1;
}

static int Lnode___index(lua_State *L) {
    ls_LuaNode *node = (ls_LuaNode*)lbind_check(L, 1, &lbT_Node);
    int idx;
    if ((idx = lua_tonumber(L, 2)) != 0 || lua_isnumber(L, 2)) {
        if (idx < 0 && -idx <= node->n)
            idx += node->n + 1;
        lua_getuservalue(L, 1);
        lua_rawgeti(L, -1, idx);
    }
    else {
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_insert(L, 1);
        lua_call(L, 2, 1);
    }
    return 1;
}

static int Lnode___len(lua_State *L) {
    ls_LuaNode *node = (ls_LuaNode*)lbind_check(L, 1, &lbT_Node);
    lua_pushinteger(L, node->n);
    return 1;
}

static int ipairsaux (lua_State *L) {
    int i = luaL_checkint(L, 2);
    if (lua_istable(L, 1)) {
        i++;  /* next value */
        lua_pushinteger(L, i);
        lua_rawgeti(L, 1, i);
        return (lua_isnil(L, -1)) ? 1 : 2;
    }
    return 0;
}

static int Lnode___ipairs(lua_State *L) {
    lua_getuservalue(L, 1);
    lua_pushcfunction(L, ipairsaux);
    lua_pushinteger(L, 0);
    return 3;
}


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
        ENTRY(new), /* for test */
        ENTRY(delete),
        ENTRY(append),
        ENTRY(insert),
        ENTRY(remove),
        ENTRY(removeself),
        { NULL, NULL }
    }, libs_meta[] = {
        ENTRY(__ipairs),
        ENTRY(__len),
#undef  ENTRY
        { NULL, NULL }
    };
    lbind_newclass_meta(L, "node", libs, NULL, &lbT_Node);
    lbind_setaccessor(L, libs_getter, NULL, &lbT_Node);
    if (lbind_getmetatable(L, &lbT_Node)) {
        lua_getfield(L, -1, "__index");
        lua_pushcclosure(L, Lnode___index, 1);
        lua_setfield(L, -2, "__index");
        lua_getfield(L, -1, "__newindex");
        lua_pushcclosure(L, Lnode___newindex, 1);
        lua_setfield(L, -2, "__newindex");
        lua_pop(L, 1);
    }
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
 * cc: libs+='d:/lua52/lua52.dll' run='lua test_node.lua'
 * cc: input+='lsnode.c lsevent.c lbind/lbind.c' output='node.dll'
 */
