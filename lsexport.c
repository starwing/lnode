#include "lsexport.h"

void **lsL_testudata_uv(lua_State *L, int narg, int uv) {
    void **p = (void**)lua_touserdata(L, narg);
    if (p == NULL) return NULL;
    lua_getmetatable(L, narg);
    if (!lua_rawequal(L, -1, lua_upvalueindex(uv)))
        p = NULL;
    lua_pop(L, 1);
    return p;
}

void *lsL_checkudata(lua_State *L, int narg, void **p) {
    if (p == NULL) {
        void **p = (void**)lua_touserdata(L, narg);
        if (p && *p == NULL)
            luaL_argerror(L, narg, "node expected, got null node");
        lua_pushfstring(L, "node expected, got %s", luaL_typename(L, narg));
        luaL_argerror(L, narg, lua_tostring(L, -1));
    }
    return *p;
}

int lsL_initudata_uv(lua_State *L, int uv_pbox, int uv_mt) {
    /* stack: obj, table */
    void **p = (void**)lua_touserdata(L, -2);
    if (p == NULL || *p == NULL) return 0;
    lua_pushvalue(L, lua_upvalueindex(uv_mt)); /* 1 */
    lua_setmetatable(L, -3); /* 1->table */
    lua_pushvalue(L, -2); /* obj->1 */
    lua_rawsetp(L, lua_upvalueindex(uv_pbox), *p); /* 1->pbox */
    if (lua_istable(L, -1))
        lua_setuservalue(L, -2); /* table->obj */
    return 1;
}

int lsL_tostring(lua_State *L, void **p, const char *tname) {
    if (!p)
        lua_pushstring(L, lua_tostring(L, 1));
    else if (*p)
        lua_pushfstring(L, "%s: %p", tname, *p);
    else
        lua_pushfstring(L, "(null %s)", tname);
    return 1;
}
