#ifndef lselem_h
#define lselem_h


#include <stddef.h>


#define LS_ATTR_ERRORS(X) \
    X(OK,      "no error") \
    X(ENOATTR, "no such attribute") \
    X(EWRITER, "error accuors in writer") \
    X(EINVALD, "invalid attribute value")

enum ls_AttrError {
#define X(a, b) LS_ATTR_##a,
    LS_ATTR_ERRORS(X)
#undef X
    LS_ATTR_ERROR_NUM
};

typedef enum ls_AttrError ls_AttrError;
typedef struct ls_AttrHolder ls_AttrHolder;
typedef const char *(*ls_Reader)(void *ud, size_t *plen);
typedef int (*ls_Writer)(void *ud, const char *s, size_t len);

typedef ls_AttrError (*ls_AttrIntegerReader)(ls_AttrHolder *holder, int attrid, long *pv);
typedef ls_AttrError (*ls_AttrIntegerWriter)(ls_AttrHolder *holder, int attrid, long nv);
typedef ls_AttrError (*ls_AttrStrReader)(ls_AttrHolder *holder, int attrid, ls_Writer f, void *ud);
typedef ls_AttrError (*ls_AttrStrWriter)(ls_AttrHolder *holder, int attrid, ls_Reader f, void *ud);


struct ls_AttrHolder {
    ls_AttrIntegerReader int_reader;
    ls_AttrIntegerWriter int_writer;
    ls_AttrStrReader str_reader;
    ls_AttrStrWriter str_writer;
};


const char *ls_attr_strerror(ls_AttrError error);
ls_AttrError ls_attr_gets (ls_AttrHolder *holder, int attrid, ls_Writer f, void *ud);
ls_AttrError ls_attr_puts (ls_AttrHolder *holder, int attrid, ls_Reader f, void *ud);
ls_AttrError ls_attr_geti (ls_AttrHolder *holder, int attrid, long *pv);
ls_AttrError ls_attr_puti (ls_AttrHolder *holder, int attrid, long  nv);


#include "lsattrlist.h"

enum ls_Attr {
#define X(a,b) LSA_##b,
    LS_ATTRS(X)
#undef X
    LS_ATTR_NUM
};

const char *ls_attr_string      (int attr);
int         ls_attr_from_string (const char *s);

int         ls_attr_isevent          (int attr);
const char *ls_attr_event_string     (int attr);
int         ls_attr_event_fromstring (const char *s);


#endif /* lselem_h */
