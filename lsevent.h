#ifndef lsevent_h
#define lsevent_h


typedef struct ls_EventSignal ls_EventSignal;
typedef struct ls_EventSlot ls_EventSlot;
typedef void ls_EventHandler(ls_EventSlot *slot, void *evtdata);

struct ls_EventSignal {
    ls_EventSlot *slots;
};

struct ls_EventSlot {
    ls_EventSlot *next, *prev;
    ls_EventSignal *signal; /* signal connected by this slot */
    ls_EventHandler *f;
    int eventid;
};

void ls_event_initsignal (ls_EventSignal *signal);
void ls_event_initslot   (ls_EventSlot *slot, int eventid, ls_EventHandler *f);
void ls_event_connect    (ls_EventSignal *signal, ls_EventSlot *newh);
void ls_event_disconnect (ls_EventSlot *slot);
void ls_event_emit       (ls_EventSignal *signal, int eventid, void *evtdata);


#endif /* lsevent_h */
