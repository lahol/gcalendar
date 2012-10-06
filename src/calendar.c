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

#include "calendar.h"
#include <time.h>

struct _CalendarWidgetPrivate {
  HolidayContext *context;
  GList *current_holidays;
};

void calendar_widget_update_holidays(CalendarWidget *calendar)
{
  guint cur_year;

  holidays_holiday_list_free(calendar->priv->current_holidays);

  gtk_calendar_get_date(GTK_CALENDAR(calendar->widget), &cur_year, NULL, NULL);
  calendar->priv->current_holidays = 
    holidays_get_holidays_for_year(calendar->priv->context,
                                   cur_year);
}

void calendar_widget_update_holiday_marks(CalendarWidget *calendar)
{
  guint cur_month;
  GList *cur;

  gtk_calendar_clear_marks(GTK_CALENDAR(calendar->widget));

  gtk_calendar_get_date(GTK_CALENDAR(calendar->widget), NULL, &cur_month, NULL);

  cur = calendar->priv->current_holidays;
  while (cur) {
    if (((Holiday*)cur->data)->date->month == cur_month+1) {
      gtk_calendar_mark_day(GTK_CALENDAR(calendar->widget),
                            ((Holiday*)cur->data)->date->day);
    }
    cur = g_list_next(cur);
  }
}

gchar *calendar_widget_get_details(GtkCalendar *widget,
                                   guint year,
                                   guint month,
                                   guint day,
                                   CalendarWidget *cal)
{
  GString *output = g_string_new("");

  GList *cur = cal->priv->current_holidays;

  while (cur) {
    if (((Holiday*)cur->data)->date->day == day &&
        ((Holiday*)cur->data)->date->month == month+1) {
      if (output->len > 0) {
        g_string_append(output, "\n");
      }
      g_string_append(output, ((Holiday*)cur->data)->description);
    }
    cur = g_list_next(cur);
  }

  if (output->len == 0) {
    g_string_free(output, TRUE);
    return NULL;
  }
  else {
    return g_string_free(output, FALSE);
  }
}

void calendar_widget_year_changed(GtkCalendar *widget,
                                 CalendarWidget *cal)
{
  g_return_if_fail(cal != NULL);

  calendar_widget_update_holidays(cal);
}

void calendar_widget_month_changed(GtkCalendar *widget,
                                   CalendarWidget *cal)
{
  g_return_if_fail(cal != NULL);

  calendar_widget_update_holiday_marks(cal);
}

void calendar_widget_weak_notify(CalendarWidget *cal,
                                 GtkWidget *widget)
{
  cal->widget = NULL;
  calendar_widget_destroy(cal);
  cal->priv = NULL;
}

CalendarWidget *calendar_widget_new(void)
{
  CalendarWidget *cal = g_new0(CalendarWidget, 1);
  cal->priv = g_new0(CalendarWidgetPrivate, 1);

  cal->widget = gtk_calendar_new();

  gtk_calendar_set_detail_func(GTK_CALENDAR(cal->widget),
                               (GtkCalendarDetailFunc)calendar_widget_get_details,
                               (gpointer)cal,
                               NULL);

  g_object_set(G_OBJECT(cal->widget),
               "show-details", FALSE,
               "show-week-numbers", TRUE,
               NULL);

  g_signal_connect(cal->widget,
                   "month-changed",
                   G_CALLBACK(calendar_widget_month_changed),
                   (gpointer)cal);

  g_signal_connect(cal->widget,
                   "next-year",
                   G_CALLBACK(calendar_widget_year_changed),
                   (gpointer)cal);

  g_signal_connect(cal->widget,
                   "prev-year",
                   G_CALLBACK(calendar_widget_year_changed),
                   (gpointer)cal);

  calendar_widget_update_holidays(cal);
  calendar_widget_update_holiday_marks(cal);

  g_object_weak_ref(G_OBJECT(cal->widget),
                    (GWeakNotify)calendar_widget_weak_notify,
                    (gpointer)cal);

  return cal;
}

void calendar_widget_set_date(CalendarWidget *calendar,
                              guint year,
                              guint month,
                              guint day)
{
}

void calendar_widget_select_today(CalendarWidget *calendar)
{
  time_t current_time;
  struct tm *today;

  time(&current_time);
  today = localtime(&current_time);
  gtk_calendar_select_month(GTK_CALENDAR(calendar->widget), today->tm_mon, today->tm_year+1900);
  gtk_calendar_select_day(GTK_CALENDAR(calendar->widget), today->tm_mday);
}

void calendar_widget_next_month(CalendarWidget *calendar)
{
  guint month, year;
  gtk_calendar_get_date(GTK_CALENDAR(calendar->widget), &year, &month, NULL);

  if (month == 11) {
    month = 0;
    ++year;
  }
  else {
    ++month;
  }

  gtk_calendar_select_month(GTK_CALENDAR(calendar->widget), month, year);
}

void calendar_widget_prev_month(CalendarWidget *calendar)
{
  guint month, year;
  gtk_calendar_get_date(GTK_CALENDAR(calendar->widget), &year, &month, NULL);

  if (month == 0) {
    month = 11;
    --year;
  }
  else {
    --month;
  }

  gtk_calendar_select_month(GTK_CALENDAR(calendar->widget), month, year);
}

void calendar_widget_next_year(CalendarWidget *calendar)
{
  guint month, year;
  gtk_calendar_get_date(GTK_CALENDAR(calendar->widget), &year, &month, NULL);

  ++year;

  gtk_calendar_select_month(GTK_CALENDAR(calendar->widget), month, year);
}

void calendar_widget_prev_year(CalendarWidget *calendar)
{
  guint month, year;
  gtk_calendar_get_date(GTK_CALENDAR(calendar->widget), &year, &month, NULL);

  --year;
  
  gtk_calendar_select_month(GTK_CALENDAR(calendar->widget), month, year);
}

void calendar_widget_destroy(CalendarWidget *calendar)
{
  holidays_holiday_list_free(calendar->priv->current_holidays);
  holidays_holiday_context_free(calendar->priv->context);
  g_free(calendar->priv);
}

void calendar_widget_set_holiday_context(CalendarWidget *calendar,
                                         HolidayContext *context)
{
  g_return_if_fail(calendar != NULL);

  if (calendar->priv->context && calendar->priv->context != context) {
    holidays_holiday_context_free(calendar->priv->context);
  }
  calendar->priv->context = context;

  calendar_widget_update_holidays(calendar);
}
