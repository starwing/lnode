#ifndef lsevent_h
#define lsevent_h


#include "lsnode.h"

typedef struct ls_EventNode ls_EventNode;
typedef struct ls_EventHandler ls_EventHandler;
typedef void (*ls_EventProc)(void *ud, void *evtdata, ls_EventNode *receiver, ls_EventHandler *self);

struct ls_EventNode {
    ls_Node node;
    ls_EventHandler *handlers;
};

struct ls_EventHandler {
    int eventid;
    ls_EventProc f;
    void *ud;
    ls_EventNode *receiver;
    ls_EventHandler *prev, *next;
};

void ls_event_initnode(ls_EventNode *node, int type);
void ls_event_inithandler(ls_EventHandler *handler, int eventid, ls_EventProc f, void *ud);
void ls_event_addhandler(ls_EventNode *node, ls_EventHandler *handler);
void ls_event_removehandler(ls_EventHandler *handler);
void ls_event_emit(ls_EventNode *node, int eventid, void *evtdata);


#endif /* lsevent_h */
