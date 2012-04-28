#include "lsattrs.h"

#include <stddef.h>
#include <string.h>

static const char *ls_attrs[LS_ATTR_NUM] = {
#define X(a,b) a,
    LS_ATTRS(X)
#undef X
};

const char *ls_attr_string(int attr) {
    if (attr < 0 || attr >= LS_ATTR_NUM)
        return NULL;
    return ls_attrs[attr];
}

int ls_attr_from_string(const char *s) {
    int begin = 0, end = LS_ATTR_NUM;
    while (begin < end) {
        int middle = (begin+end)>>1;
        int res = strcmp(s, ls_attrs[middle]);
             if (res < 0)
            end = middle;
        else if (res > 0)
            begin = middle + 1;
        else
            return middle;
    }
    return -1;
}

int ls_attr_isevent(int attr) {
    const char *s = ls_attr_event_string(attr);
    return s ? attr : -1;
}

const char *ls_attr_event_string(int attr) {
    const char *s = ls_attr_string(attr);
    if (s && s[0] == 'o' && s[1] == 'n')
        return s;
    return NULL;
}

int ls_attr_event_fromstring(const char *s) {
    char buff[32] = "on";
    strncat(buff, s, 31);
    return ls_attr_from_string(buff);
}

#ifdef LS_EXPORT_ATTR
#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

static int Ltostring(lua_State *L) {
    int attr = luaL_checkint(L, 1);
    lua_pushstring(L, ls_attr_string(attr));
    return 1;
}

static int Lfromstring(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    int attr = ls_attr_from_string(s);
    if (attr >= 0)
        lua_pushinteger(L, attr);
    else
        lua_pushnil(L);
    return 1;
}

static int Lisevent(lua_State *L) {
    switch (lua_type(L, 1)) {
    case LUA_TNUMBER:
        lua_pushboolean(L, ls_attr_isevent(lua_tointeger(L, 1)) >= 0);
        return 1;
    case LUA_TSTRING:
        lua_pushboolean(L, ls_attr_event_fromstring(lua_tostring(L, 1)) >= 0);
        return 1;
    default:
        lua_pushfstring(L, "number/string expected, got %s", luaL_typename(L, 1));
        return luaL_argerror(L, 1, lua_tostring(L, -1));
    }
}

static int Levent_fromstring(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    int attr = ls_attr_event_fromstring(s);
    if (attr >= 0)
        lua_pushinteger(L, attr);
    else
        lua_pushnil(L);
    return 1;
}

static int Levent_tostring(lua_State *L) {
    int attr = luaL_checkint(L, 1);
    lua_pushstring(L, ls_attr_event_string(attr));
    return 1;
}

LUALIB_API int luaopen_node_attr(lua_State *L) {
    luaL_Reg attrlib[] = {
#define ENTRY(n) { #n, L##n },
        ENTRY(tostring)
        ENTRY(fromstring)
        ENTRY(isevent)
        ENTRY(event_tostring)
        ENTRY(event_fromstring)
#undef  ENTRY
        { NULL, NULL }
    };
    luaL_newlib(L, attrlib);
    lua_createtable(L, 0, LS_ATTR_NUM);
#define X(a,b) (lua_pushliteral(L, a), lua_pushvalue(L, -1), \
        lua_pushinteger(L, LSA_##b), lua_rawset(L, -4), lua_rawseti(L, -2, LSA_##b));
    LS_ATTRS(X)
#undef  X
    lua_setfield(L, -2, "attrs");
    return 1;
}
#endif /* LS_EXPORT_ATTR */
/* 
 * cc: flags+='-s -O2 -mdll -pedantic -DLUA_BUILD_AS_DLL -Id:/lua52/include'
 * cc: flags+='-DLS_EXPORT_NODE -DLS_EXPORT_ATTR'
 * cc: libs+='d:/lua52/lua52.dll'
 * cc: input='*.c' output='c-node.dll'
 */
