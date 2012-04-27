#include "lsevent.h"

#include <stddef.h>

void ls_event_inithandler(ls_EventHandler *handler, int eventid, ls_EventProc f, void *ud) {
    handler->eventid = eventid;
    handler->f = f;
    handler->ud = ud;
    handler->receiver = NULL;
    handler->prev = handler->next = handler;
}

void ls_event_addhandler(ls_EventNode *node, ls_EventHandler *handler) {
    if (handler->receiver != NULL)
        ls_event_removehandler(handler);
    if (node->handlers == NULL)
        node->handlers = handler;
    else {
        ls_EventHandler *head = node->handlers;
        head->prev->next = handler;
        handler->prev = head->prev;
        handler->next = head;
        head->prev = handler;
    }
    handler->receiver = node;
}

void ls_event_removehandler(ls_EventHandler *handler) {
    int isempty = (handler == handler->next);
    ls_EventNode *receiver = handler->receiver;
    if (receiver) {
        if (receiver->handlers == handler)
            receiver->handlers = isempty ? NULL : handler->next;
        if (!isempty) {
            handler->prev->next = handler->next;
            handler->next->prev = handler->prev;
            handler->prev = handler->next = handler;
        }
        handler->receiver = NULL;
    }
}

void ls_event_emit(ls_EventNode *node, int eventid, void *evtdata) {
    ls_EventHandler *handler, *next, *firstmatch = NULL;
    if ((handler = node->handlers)) {
        do {
            next = handler->next;
            if (handler->eventid == eventid) {
                if (firstmatch == NULL)
                    firstmatch = handler;
                handler->f(handler->ud, evtdata, node, handler);
            }
        } while ((handler = next) != node->handlers);
        node->handlers = firstmatch;
    }
}

