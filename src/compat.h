/* Provides replacements of functions an macros that are not 
 * available in older versions of glib */
#ifndef __COMPAT_H__
#define __COMPAT_H__

#include <glib.h>

#if !GLIB_CHECK_VERSION(2, 28, 0)

#define COMPAT_G_LIST_FREE_FULL
void g_list_free_full(GList *list,
                      GDestroyNotify free_func);

#endif

#endif
