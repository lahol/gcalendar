#include "compat.h"

#ifdef COMPAT_G_LIST_FREE_FULL
void g_list_free_full(GList *list,
                      GDestroyNotify free_func)
{
  GList *tmp;
  if (free_func) {
    tmp = list;
    while (tmp) {
      free_func(tmp->data);
      tmp = tmp->next;
    }
  }
  g_list_free(list);
}
#endif
