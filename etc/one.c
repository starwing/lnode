#include "../lsattrs.c"
#include "../lsbridge.c"
#include "../lsevent.c"
#include "../lsnode.c"
#include "../lbind/lbind.c"

/* cc: flags+='-s -O2 -mdll -DLUA_BUILD_AS_DLL'
 * cc: libs+='d:/lua52/lua52.dll' run='lua test_node.lua'
 * cc: output='node.dll'
 */
