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
    ls_Node *parent, *firstchild;
    if ((parent = self->parent) && parent->children == self) /* first child */
        return parent;
    self = self->prev_sibling;
    while ((firstchild = self->children))
        self = firstchild->prev_sibling;
    return self;
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

