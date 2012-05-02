#ifndef lsevent_h
#define lsevent_h


typedef struct ls_EventSlot ls_EventSlot;
typedef struct ls_EventHandler ls_EventHandler;
typedef void (*ls_EventProc)(void *ud, void *evtdata, ls_EventSlot *slot, ls_EventHandler *self);

struct ls_EventSlot {
    ls_EventHandler *handlers;
};

struct ls_EventHandler {
    int eventid;
    ls_EventProc f;
    void *ud;
    ls_EventSlot *slot;
    ls_EventHandler *prev, *next;
};

void ls_event_inithandler   (ls_EventHandler *handler, int eventid, ls_EventProc f, void *ud);
void ls_event_removehandler (ls_EventHandler *handler);
void ls_event_emit          (ls_EventSlot *slot, int eventid, void *evtdata);
void ls_event_addhandler    (ls_EventSlot *slot, ls_EventHandler *newh);


#endif /* lsevent_h */
