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
