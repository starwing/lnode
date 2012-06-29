#ifndef lsevent_h
#define lsevent_h


typedef struct ls_EventSignal ls_EventSignal;
typedef struct ls_EventSlot ls_EventSlot;
typedef void (*ls_EventHandler)(void *ud, void *evtdata, ls_EventSignal *signal, ls_EventSlot *self);

struct ls_EventSignal {
    ls_EventSlot *slots;
};

struct ls_EventSlot {
    ls_EventSlot *next, *prev;
    int eventid;
    ls_EventHandler f;
    void *ud;
    ls_EventSignal *signal;
};

void ls_event_initsignal (ls_EventSignal *signal);
void ls_event_initslot   (ls_EventSlot *slot, int eventid, ls_EventHandler f, void *ud);
void ls_event_connect    (ls_EventSignal *signal, ls_EventSlot *newh);
void ls_event_disconnect (ls_EventSlot *slot);
void ls_event_emit       (ls_EventSignal *signal, int eventid, void *evtdata);


#endif /* lsevent_h */
