#ifndef __HOLIDAYS_H__
#define __HOLIDAYS_H__

#include <glib.h>

typedef struct _Holiday {
  GDate *date;
  gchar *description;
} Holiday;

typedef struct _HolidayContext HolidayContext;

HolidayContext *holidays_holiday_context_new(const gchar *filename);

void holidays_holiday_context_free(HolidayContext *context);

GList *holidays_get_holidays_for_year(HolidayContext *context,
                                      guint year);

void holidays_holiday_list_free(GList *holiday_list);

#endif
