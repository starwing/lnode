#include "lsnode.h"

#include <assert.h>
#include <stddef.h>

#define list_for_each(pos, list) \
    for (pos = (list)->next_sibling; pos != (list); pos = pos->next_sibling)

#define list_for_each_safe(pos, n, list) \
    for (pos = (list)->next_sibling, n = pos->next_sibling; \
            pos != (list); pos = n, n = n->next_sibling)

void ls_initnode(ls_Node *self, int type) {
    self->type = type;
    self->parent = NULL;
    self->prev_sibling = self->next_sibling = self;
    self->children = NULL;
}

void ls_append(ls_Node *self, ls_Node *newNode) {
    if (newNode->next_sibling != newNode) ls_removeself(newNode);

    newNode->prev_sibling = self->prev_sibling;
    newNode->next_sibling = self;
    self->prev_sibling->next_sibling = newNode;
    self->prev_sibling = newNode;

    newNode->parent = self->parent;
}

void ls_insert(ls_Node *self, ls_Node *newNode) {
    ls_append(self, newNode);
    if (self->parent && self->parent->children == self)
        self->parent->children = newNode;
}

void ls_setchildren(ls_Node *self, ls_Node *newNode) {
    ls_Node *i;
    if (self->children != NULL) {
        self->children->parent = NULL;
        list_for_each (i, self->children)
            i->parent = NULL;
    }
    if (newNode) {
        if (newNode->parent)
            newNode->parent->children = NULL;
        newNode->parent = self;
        list_for_each (i, newNode)
            i->parent = self;
    }
    self->children = newNode;
}

void ls_removeself(ls_Node *self) {
    if (self == self->next_sibling && self == self->prev_sibling) {
        if (self->parent) {
            assert(self->parent->children == self);
            self->parent->children = NULL;
        }
    }
    else {
        self->prev_sibling->next_sibling = self->next_sibling;
        self->next_sibling->prev_sibling = self->prev_sibling;
        if (self->parent && self->parent->children == self) {
            self->parent->children = self->next_sibling;
        }
        self->prev_sibling = self->next_sibling = self;
    }
    self->parent = NULL;
}

int      ls_type        (ls_Node *self) { return self->type; }
ls_Node* ls_parent      (ls_Node *self) { return self->parent; }
ls_Node* ls_prevsibling (ls_Node *self) { return self->prev_sibling; }
ls_Node* ls_nextsibling (ls_Node *self) { return self->next_sibling; }
ls_Node* ls_firstchild  (ls_Node *self) { return self->children; }
ls_Node* ls_lastchild   (ls_Node *self) { return self->children ? self->children->prev_sibling : NULL; }

ls_Node* ls_root(ls_Node *self) {
    ls_Node *parent;
    while ((parent = self->parent) != NULL)
        self = parent;
    return self;
}

ls_Node* ls_firstleaf(ls_Node *root) {
    return root;
}

ls_Node* ls_lastleaf(ls_Node *root) {
    ls_Node *firstchild;
    while ((firstchild = root->children) != NULL)
        root = firstchild->prev_sibling;
    return root;
}

ls_Node* ls_prevleaf(ls_Node *self) {
    if (self->parent && self->parent->children == self) /* first child */
        return self->parent;
    return ls_lastleaf(self->prev_sibling);
}

ls_Node* ls_nextleaf(ls_Node *self) {
    ls_Node *parent;
    if (self->children)
        return self->children;
    while ((parent = self->parent)
            && parent->children == self->next_sibling)
        self = parent;
    return self->next_sibling;
}

#ifdef LS_EXPORT_NODE
#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

static int Lnew(lua_State *L) {
    struct ls_NodeWithPtr {
        ls_Node *p;
        ls_Node node;
    } *nwp = (struct ls_NodeWithPtr*)lua_newuserdata(L,
            sizeof(struct ls_NodeWithPtr));
    ls_Node *p = nwp->p = &nwp->node;
    ls_initnode(p, 0);
    lua_pushvalue(L, lua_upvalueindex(2));
    lua_setmetatable(L, -2);
    lua_pushvalue(L, -1);
    lua_rawsetp(L, lua_upvalueindex(1), p);
    if (lua_istable(L, 1)) {
        lua_pushvalue(L, 1);
        lua_setuservalue(L, -2);
    }
    return 1;
}

static ls_Node **test_node(lua_State *L, int narg) {
    ls_Node **p = (ls_Node**)lua_touserdata(L, narg);
    if (p == NULL) return NULL;
    lua_getmetatable(L, narg);
    if (!lua_rawequal(L, -1, lua_upvalueindex(2)))
        return NULL;
    lua_pop(L, 1);
    return p;
}

static int Ldelete(lua_State *L) {
    ls_Node **p = test_node(L, 1);
    if (p && *p) {
        ls_removeself(*p);
        *p = NULL;
        lua_pushnil(L);
        lua_setmetatable(L, 1);
        lua_pushnil(L);
        lua_setuservalue(L, 1);
    }
    return 0;
}

static int Lisnode(lua_State *L) {
    lua_pushboolean(L, test_node(L, 1) != NULL);
    return 1;
}

static ls_Node *check_node(lua_State *L, int narg) {
    ls_Node **p = test_node(L, narg);
    if (p == NULL) {
        p = (ls_Node**)lua_touserdata(L, narg);
        if (p && *p == NULL)
            luaL_argerror(L, narg, "node expected, got null node");
        lua_pushfstring(L, "node expected, got %s", luaL_typename(L, narg));
        luaL_argerror(L, narg, lua_tostring(L, -1));
    }
    return *p;
}

static int Lutable(lua_State *L) {
    check_node(L, 1);
    lua_getuservalue(L, 1);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setuservalue(L, 1);
    }
    return 1;
}

static int Lappend(lua_State *L) {
    if (!lua_isnil(L, 1)) {
        ls_Node *node = check_node(L, 1);
        ls_Node *newNode = check_node(L, 2);
        ls_append(node, newNode);
        lua_settop(L, 1);
    } /* else, return argument 2 */
    return 1;
}

static int Linsert(lua_State *L) {
    if (!lua_isnil(L, 1)) {
        ls_Node *node = check_node(L, 1);
        ls_Node *newNode = check_node(L, 2);
        ls_insert(node, newNode);
        lua_settop(L, 1);
    } /* else return argument 2 */
    return 1;
}

static int Lsetchildren(lua_State *L) {
    ls_Node *node = check_node(L, 1);
    if (!lua_isnil(L, 2)) {
        ls_Node *newNode = check_node(L, 2);
        ls_setchildren(node, newNode);
    }
    else {
        ls_setchildren(node, NULL);
    }
    lua_settop(L, 1);
    return 1;
}

static int Lremoveself(lua_State *L) {
    ls_Node *node = check_node(L, 1);
    ls_removeself(node);
    return 0;
}

static int Ltype(lua_State *L) {
    ls_Node *node = check_node(L, 1);
    lua_pushinteger(L, ls_type(node));
    return 1;
}

#define BIND_UNARG_FUNC(name) \
    static int L##name(lua_State *L) {                              \
        ls_Node *node = check_node(L, 1);                           \
        ls_Node *result = ls_##name(node);                          \
        if (result == NULL) {                                       \
            lua_pushnil(L);                                         \
            return 1;                                               \
        }                                                           \
        lua_rawgetp(L, lua_upvalueindex(1), result);                \
        if (lua_isnil(L, -1)) {                                     \
            ls_Node **node =                                        \
                (ls_Node**)lua_newuserdata(L, sizeof(ls_Node*));    \
            lua_remove(L, -2);                                      \
            *node = result;                                         \
            lua_pushvalue(L, lua_upvalueindex(2));                  \
            lua_setmetatable(L, -2);                                \
            lua_pushvalue(L, -1);                                   \
            lua_rawsetp(L, lua_upvalueindex(1), result);            \
        }                                                           \
        return 1;                                                   \
    }
BIND_UNARG_FUNC(parent)
BIND_UNARG_FUNC(prevsibling)
BIND_UNARG_FUNC(nextsibling)
BIND_UNARG_FUNC(firstchild)
BIND_UNARG_FUNC(lastchild)
BIND_UNARG_FUNC(root)
BIND_UNARG_FUNC(firstleaf)
BIND_UNARG_FUNC(lastleaf)
BIND_UNARG_FUNC(prevleaf)
BIND_UNARG_FUNC(nextleaf)
#undef BIND_UNARG_FUNC

static int Ltostring(lua_State *L) {
    ls_Node **p = test_node(L, 1);
    if (!p)
        lua_pushstring(L, lua_tostring(L, 1));
    else if (*p)
        lua_pushfstring(L, "node: %p", *p);
    else
        lua_pushstring(L, "(null node)");
    return 1;
}

static int Linit(lua_State *L) {
    luaL_Reg nodelibs[] = {
#define ENTRY(n) { #n, L##n }
        ENTRY(new),
        ENTRY(delete),
        ENTRY(isnode),
        ENTRY(utable),
        ENTRY(append),
        ENTRY(insert),
        ENTRY(setchildren),
        ENTRY(removeself),
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
        ENTRY(tostring),
#undef ENTRY
        { NULL, NULL }
    };
    if (lua_gettop(L) < 2)
        luaL_argerror(L, 1, "(pbox, metatable) expected");
    lua_newtable(L);
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    luaL_setfuncs(L, nodelibs, 2);
    return 1;
}

LUALIB_API int luaopen_node(lua_State *L) {
    lua_pushcfunction(L, Linit);
    return 1;
}
#endif /* LS_EXPORT_NODE */
/* 
 * cc: flags+='-s -O2 -mdll -pedantic -DLUA_BUILD_AS_DLL -IC:/lua52/include -DLS_EXPORT_NODE'
 * cc: libs+='c:/lua52/lua52.dll'
 * cc: output='c-node.dll'
 */