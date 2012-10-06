/*  gcalendar -- a calendar tray widget
 *  Copyright (C) 2012 Holger Langenau
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

void calendar_widget_select_today(CalendarWidget *calendar);

void calendar_widget_next_month(CalendarWidget *calendar);

void calendar_widget_prev_month(CalendarWidget *calendar);

void calendar_widget_next_year(CalendarWidget *calendar);

void calendar_widget_prev_year(CalendarWidget *calendar);

void calendar_widget_destroy(CalendarWidget *calendar);

void calendar_widget_set_holiday_context(CalendarWidget *calendar,
                                         HolidayContext *context);

#endif
