#ifndef __CALENDAR_H__
#define __CALENDAR_H__

#include <gtk/gtk.h>
#include "holidays.h"

typedef struct _CalendarWidget CalendarWidget;
typedef struct _CalendarWidgetPrivate CalendarWidgetPrivate;

struct _CalendarWidget {
  GtkWidget *widget;

  CalendarWidgetPrivate *priv;
};

CalendarWidget *calendar_widget_new(void);

void calendar_widget_set_date(CalendarWidget *calendar,
                              guint year,
                              guint month,
                              guint day);

void calendar_widget_destroy(CalendarWidget *calendar);

void calendar_widget_set_holiday_context(CalendarWidget *calendar,
                                         HolidayContext *context);

#endif
