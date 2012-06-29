#include "lsevent.h"


#include <assert.h>
#include <stddef.h>


void ls_event_initsignal(ls_EventSignal *signal) {
    signal->slots = NULL;
}

void ls_event_initslot(ls_EventSlot *slot, int eventid, ls_EventHandler *f) {
    slot->prev = slot->next = slot;
    slot->eventid = eventid;
    slot->f = f;
    slot->signal = NULL;
}

void ls_event_connect(ls_EventSignal *signal, ls_EventSlot *slot) {
    if (slot->signal != NULL)
        ls_event_disconnect(slot);
    if (signal->slots == NULL)
        signal->slots = slot;
    else {
        ls_EventSlot *head = signal->slots;
        head->prev->next = slot;
        slot->prev = head->prev;
        slot->next = head;
        head->prev = slot;
    }
    slot->signal = signal;
}

void ls_event_disconnect(ls_EventSlot *slot) {
    int isempty = (slot == slot->next);
    ls_EventSignal *signal = slot->signal;
    if (signal) {
        if (signal->slots == slot)
            signal->slots = isempty ? NULL : slot->next;
        if (!isempty) {
            slot->prev->next = slot->next;
            slot->next->prev = slot->prev;
            slot->prev = slot->next = slot;
        }
        slot->signal = NULL;
    }
}

void ls_event_emit(ls_EventSignal *signal, int eventid, void *evtdata) {
    ls_EventSlot *slot, *next, *firstmatch = NULL;
    if ((slot = signal->slots)) {
        do {
            next = slot->next;
            assert(slot->signal == signal);
            if (slot->eventid == eventid) {
                if (firstmatch == NULL)
                    firstmatch = slot;
                slot->f(slot, evtdata);
            }
        } while ((slot = next) != signal->slots);
        signal->slots = firstmatch;
    }
}

