#ifndef lsbridge_h
#define lsbridge_h

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lsattrs.h"
#include "lsevent.h"
#include "lsnode.h"

typedef struct ls_LuaNode ls_LuaNode;
typedef struct ls_LuaEventSlot ls_LuaEventSlot;


ls_LuaNode      *lsL_node_new  (lua_State *L, size_t nodesize);
ls_LuaEventSlot *lsL_event_new (lua_State *L, size_t slotsize);

ls_Node      *lsL_node (ls_LuaNode *node);
ls_EventSlot *lsL_slot (ls_LuaEventSlot *event);

void lsL_event_register   (lua_State *L, ls_EventSignal *signal);
void lsL_event_unregister (lua_State *L, ls_EventSignal *signal);

int lsL_event_emit (lua_State *L, ls_EventSignal *signal, int evtid, int args);

void lsL_attrs_register   (lua_State *L, ls_AttrHolder *holder);
void lsL_attrs_unregister (lua_State *L, ls_AttrHolder *holder);

int  lsL_get_attrs_table (lua_State *L, ls_AttrHolder *holder);
void lsL_set_attrs_table (lua_State *L, int idx, ls_AttrHolder *holder);

LUALIB_API int luaopen_bridge(lua_State *L);

#endif /* lsbridge_h */
