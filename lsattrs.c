#include "lsattrs.h"

#include <stddef.h>
#include <string.h>

static const char *ls_attr_errors[LS_ATTR_ERROR_NUM] = {
#define X(a, b) b,
    LS_ATTR_ERRORS(X)
#undef  X
};

const char *ls_attr_strerror(ls_AttrError error) {
    int i = (int)error;
    if (i >= 0 && i < LS_ATTR_ERROR_NUM)
        return ls_attr_errors[i];
    return "unknown error";
}

ls_AttrError ls_attr_gets(ls_AttrHolder *holder, int attrid, ls_Writer f, void *ud) {
    if (!holder->str_reader)
        return LS_ATTR_ENOATTR;
    return holder->str_reader(holder, attrid, f, ud);
}

ls_AttrError ls_attr_puts(ls_AttrHolder *holder, int attrid, ls_Reader f, void *ud) {
    if (!holder->str_writer)
        return LS_ATTR_ENOATTR;
    return holder->str_writer(holder, attrid, f, ud);
}

ls_AttrError ls_attr_geti(ls_AttrHolder *holder, int attrid, long *pv) {
    if (!holder->int_reader)
        return LS_ATTR_ENOATTR;
    return holder->int_reader(holder, attrid, pv);
}

ls_AttrError ls_attr_puti(ls_AttrHolder *holder, int attrid, long nv) {
    if (!holder->int_reader)
        return LS_ATTR_ENOATTR;
    return holder->int_writer(holder, attrid, nv);
}

