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

#include "holidays.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct _HolidayContextEntry {
  void (*relative)(GDate *);
  gint offset;
  guint day;
  guint month;
  gchar *description;
};

struct _HolidayContext {
  GList *holidays;
};

struct HolidayParserState {
  GString *content;
  HolidayContext *context;
  struct _HolidayContextEntry *current;
};

void holidays_holiday_free(Holiday *holiday)
{
  g_date_free(holiday->date);
/*  g_free(holiday->description);*/
}

guint holidays_get_day_of_year(guint year, guint month, guint day)
{
  guint doy = 0;
  if (month >= 8) {
    doy = 214;
    month -= 7;
  }
  doy += (month/2)*31 + ((month-1)/2)*30;
  if (month >= 3) {
    if (g_date_is_leap_year(year)) {
      doy += 1;
    }
    doy -= 2;
  }

  return doy + day;
}

void holidays_set_day_of_year(GDate *date, guint doy)
{
  guint day = doy;
  guint month = 1;
  guint max = g_date_get_days_in_month(1, date->year);

  while (day > max) {
    day -= max;
    month++;
    max = g_date_get_days_in_month(month, date->year);
  }

  g_date_set_dmy(date, day, month, date->year);
}

guint holidays_get_day_of_week(guint year,
                               guint month,
                               guint day)
{
  guint doy = holidays_get_day_of_year(year, month, day);
  guint dow = year + year/4 + (year/100)/4 + doy - 1 - year/100;
  if (g_date_is_leap_year(year)) {
    dow -= 1;
  }
  return (dow % 7);
}

void holidays_get_fourth_advent(GDate *date)
{
  guint w2412 = holidays_get_day_of_week(date->year, 12, 24);
  g_date_set_dmy(date, 24-w2412, G_DATE_DECEMBER, date->year);
}

void holidays_get_easter_sunday(GDate *date)
{
  guint d = (((255-11*(date->year%19))-21)%30)+21;
  guint c = d > 48 ? 1 : 0;
  guint yday = holidays_get_day_of_year(date->year, 3, 1); 
  yday += (d+c+6-((date->year+date->year/4)+d+c+1)%7);
  holidays_set_day_of_year(date, yday);
}

void holidays_get_mothers_day(GDate *date)
{
  guint dow = holidays_get_day_of_year(date->year, 5, 1);
  g_date_set_dmy(date, dow == 0 ? 8 : 15-dow, G_DATE_MAY, date->year);
}

void holidays_get_begin_dst(GDate *date)
{
  g_date_set_dmy(date,
                 31-holidays_get_day_of_week(date->year, 3, 31),
                 G_DATE_MARCH,
                 date->year);
}

void holidays_get_end_dst(GDate *date)
{
  g_date_set_dmy(date,
                 31-holidays_get_day_of_week(date->year, 10, 31),
                 G_DATE_OCTOBER,
                 date->year);
}

gint holidays_holiday_compare(Holiday *a, Holiday *b)
{
  if (!a) return b ? -1 : 0;
  if (!b) return 1;

  return g_date_compare(a->date, b->date);
}

GList *holidays_holiday_list_insert_relative(GList *list,
                                             GDate *base,
                                             gint offset,
                                             gchar *description)
{
  Holiday *holiday = g_new0(Holiday, 1);

  holiday->date = g_date_new_dmy(base->day, base->month, base->year);
  
  guint doy = g_date_get_day_of_year(holiday->date);
  holidays_set_day_of_year(holiday->date, doy+offset);

  holiday->description = description;

  return g_list_insert_sorted(list,
                              (gpointer)holiday,
                              (GCompareFunc)holidays_holiday_compare);
}

#define holidays_holiday_list_insert(l,b,d) (holidays_holiday_list_insert_relative((l),(b),0,(d)))

GList *holidays_holiday_list_insert_dmy(GList *list,
                                        guint day,
                                        guint month,
                                        guint year,
                                        gchar *description)
{
  GList *tmp;
  GDate *date = g_date_new_dmy(day, month, year);
  tmp = holidays_holiday_list_insert(list, date, description);
  g_date_free(date);
  return tmp;
}

GList *holidays_get_holidays_for_year(HolidayContext *context,
                                      guint year)
{
  GList *holidays = NULL;
  GDate *date = NULL;
  GList *cur = NULL;
  struct _HolidayContextEntry *entry;

  void (*get_relative)(GDate *) = NULL;

  if (!context) {
    return NULL;
  }

  date = g_date_new_dmy(1, 1, year);

  cur = context->holidays;
  while (cur) {
    entry = cur->data;

    if (get_relative != entry->relative) {
      get_relative = entry->relative;
      if (get_relative) {
        get_relative(date);
      }
    }

    if (get_relative) {
      holidays = holidays_holiday_list_insert_relative(holidays, date, entry->offset, entry->description);
    }
    else {
      g_date_set_day(date, entry->day);
      g_date_set_month(date, entry->month);
      holidays = holidays_holiday_list_insert(holidays, date, entry->description);
    }

    cur = g_list_next(cur);
  }

  g_date_free(date);
  return holidays;
}

void holidays_holiday_list_free(GList *holiday_list)
{
  g_list_free_full(holiday_list,
                   (GDestroyNotify)holidays_holiday_free);
}

void holidays_holiday_context_parser_start_element(GMarkupParseContext *parse_ctx,
                                                   const gchar *element_name,
                                                   const gchar **attribute_names,
                                                   const gchar **attribute_values,
                                                   gpointer user_data,
                                                   GError **error)
{
  struct HolidayParserState *state = user_data;
  gchar *relative = NULL, *offset = NULL, *day = NULL, *month = NULL;

  if (strcmp(element_name, "holiday") == 0) {
    state->current = g_new0(struct _HolidayContextEntry, 1);
    if (!state->content) {
      state->content = g_string_new("");
    }
    if (!g_markup_collect_attributes(element_name,
          attribute_names, attribute_values, NULL,
          G_MARKUP_COLLECT_STRING, "relative", &relative,
          G_MARKUP_COLLECT_STRING, "offset", &offset,
          G_MARKUP_COLLECT_INVALID)) {
      g_markup_collect_attributes(element_name,
        attribute_names, attribute_values, NULL,
        G_MARKUP_COLLECT_STRING, "day", &day,
        G_MARKUP_COLLECT_STRING, "month", &month,
        G_MARKUP_COLLECT_INVALID);
    }

    if (relative && offset) {
      if (strcmp(relative, "easter sunday") == 0) {
        state->current->relative = holidays_get_easter_sunday;
      }
      else if (strcmp(relative, "fourth advent") == 0) {
        state->current->relative = holidays_get_fourth_advent;
      }
      else if (strcmp(relative, "mothers day") == 0) {
        state->current->relative = holidays_get_mothers_day;
      }
      else if (strcmp(relative, "begin dst") == 0) {
        state->current->relative = holidays_get_begin_dst;
      }
      else if (strcmp(relative, "end dst") == 0) {
        state->current->relative = holidays_get_end_dst;
      }
      state->current->offset = strtol(offset, NULL, 10);
    }
    else if (day && month) {
      state->current->day = strtoul(day, NULL, 10);
      state->current->month = strtoul(month, NULL, 10);
    }
  }
}

void holidays_holiday_context_parser_end_element(GMarkupParseContext *parse_ctx,
                                                 const gchar *element_name,
                                                 gpointer user_data,
                                                 GError **error)
{
  struct HolidayParserState *state = user_data;
  gchar *content = NULL;

  if (strcmp(element_name, "holiday") == 0) {
    if (state->content) {
      content = g_string_free(state->content, FALSE);
      state->content = NULL;
      g_strstrip(content);
    }
    state->current->description = content;

    state->context->holidays = g_list_prepend(state->context->holidays, state->current);
    state->current = NULL;
  }
}

void holidays_holiday_context_parser_text(GMarkupParseContext *parse_ctx,
                                          const gchar *text,
                                          gsize text_len,
                                          gpointer user_data,
                                          GError **error)
{
  struct HolidayParserState *state = user_data;
  if (state->content) {
    g_string_append_len(state->content, text, text_len);
  }
}

/* Helper to sort the list in descending order */
gint holidays_holiday_context_entry_compare(struct _HolidayContextEntry *a,
                                            struct _HolidayContextEntry *b)
{
  if (!a) return b ? 1 : 0;
  if (!b) return -1;

  /* group the holidays according to their depending date, i.e. all holidays
   * relative to easter sunday are consecutively ordered, as are the fixed
   * dates. This makes sure that we only calculate the main dates once per year.
   * That is why we compare function pointers. */
  if (GPOINTER_TO_UINT(a->relative) < GPOINTER_TO_UINT(b->relative)) {
    return 1;
  }
  else if (GPOINTER_TO_UINT(a->relative) > GPOINTER_TO_UINT(b->relative)) {
    return -1;
  }
  else {
    if (a->relative != NULL) {
      if (a->offset < b->offset) {
        return 1;
      }
      else if (a->offset > b->offset) {
        return -1;
      }
    }
    else {
      if (a->month < b->month) {
        return 1;
      }
      else if (a->month > b->month) {
        return -1;
      }
      else {
        if (a->day < b->day) {
          return 1;
        }
        else if (a->day > b->day) {
          return -11;
        }
      }
    }
  }
  return 0;
}

HolidayContext *holidays_holiday_context_new(const gchar *filename)
{
  FILE *f;
  GMarkupParser parser = { NULL, };
  GMarkupParseContext *parse_ctx = NULL;
  GError *error = NULL;
  gchar buffer[512];
  size_t bytes;

  struct HolidayParserState state;
  state.content = NULL;
  state.current = NULL;

  if (!filename) {
    return NULL;
  }

  if ((f = fopen(filename, "r")) == NULL) {
    return NULL;
  }
  
  state.context = g_new0(HolidayContext, 1);

  parser.start_element = holidays_holiday_context_parser_start_element;
  parser.end_element   = holidays_holiday_context_parser_end_element;
  parser.text          = holidays_holiday_context_parser_text;

  parse_ctx = g_markup_parse_context_new(&parser,
                                         G_MARKUP_TREAT_CDATA_AS_TEXT,
                                         &state,
                                         NULL);

  while (!feof(f) && (bytes = fread(buffer, 1, 512, f)) > 0) {
    if (!g_markup_parse_context_parse(parse_ctx,
                                      buffer,
                                      bytes,
                                      &error)) {
      g_print("break with %zd bytes\n", bytes);
      break;
    }
  }

  fclose(f);

  if (error || !g_markup_parse_context_end_parse(parse_ctx, &error)) {
    g_warning("Unable to parse file: %s\n", error->message);
    g_error_free(error);
    g_markup_parse_context_free(parse_ctx);
    holidays_holiday_context_free(state.context);
    if (state.content) {
      g_string_free(state.content, TRUE);
    }
    return NULL;
  }

  g_markup_parse_context_free(parse_ctx);

  state.context->holidays = g_list_sort(state.context->holidays,
                                        (GCompareFunc)holidays_holiday_context_entry_compare);

  return state.context;
}

void holidays_holiday_context_free(HolidayContext *context)
{
  GList *cur;
  if (context) {
    cur = context->holidays;
    while (cur) {
      g_free(((struct _HolidayContextEntry*)cur->data)->description);
      g_free(cur->data);
      cur = g_list_next(cur);
    }
    g_list_free(context->holidays);
    g_free(context);
  }
}
