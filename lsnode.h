#ifndef lsnode_h
#define lsnode_h

typedef struct ls_Node ls_Node;

struct ls_Node {
    int type;
    ls_Node *parent;
    ls_Node *prev_sibling;
    ls_Node *next_sibling;
    ls_Node *children;
};

void ls_initnode (ls_Node *self, int type);

void ls_append (ls_Node *self, ls_Node *newNode);
void ls_insert (ls_Node *self, ls_Node *newNode);

void ls_setchildren (ls_Node *self, ls_Node *children);
void ls_removeself  (ls_Node *self);

int      ls_type        (ls_Node *self);
ls_Node* ls_parent      (ls_Node *self);
ls_Node* ls_prevsibling (ls_Node *self);
ls_Node* ls_nextsibling (ls_Node *self);
ls_Node* ls_firstchild  (ls_Node *self);
ls_Node* ls_lastchild   (ls_Node *self);

ls_Node* ls_root      (ls_Node *self);
ls_Node* ls_firstleaf (ls_Node *root);
ls_Node* ls_lastleaf  (ls_Node *root);
ls_Node* ls_prevleaf  (ls_Node *self);
ls_Node* ls_nextleaf  (ls_Node *self);

#endif /* lsnode_h */
