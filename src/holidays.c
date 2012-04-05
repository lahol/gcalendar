#include "holidays.h"

struct _HolidayContext {
};

void holidays_holiday_free(Holiday *holiday)
{
  g_date_free(holiday->date);
  g_free(holiday->description);
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
                                             const gchar *description)
{
  Holiday *holiday = g_new0(Holiday, 1);

  holiday->date = g_date_new_dmy(base->day, base->month, base->year);
  
  guint doy = g_date_get_day_of_year(holiday->date);
  holidays_set_day_of_year(holiday->date, doy+offset);

  holiday->description = g_strdup(description);

  return g_list_insert_sorted(list,
                              (gpointer)holiday,
                              (GCompareFunc)holidays_holiday_compare);
}

#define holidays_holiday_list_insert(l,b,d) (holidays_holiday_list_insert_relative((l),(b),0,(d)))

GList *holidays_holiday_list_insert_dmy(GList *list,
                                        guint day,
                                        guint month,
                                        guint year,
                                        const gchar *description)
{
  GList *tmp;
  GDate *date = g_date_new_dmy(day, month, year);
  tmp = holidays_holiday_list_insert(list, date, description);
  g_date_free(date);
  return tmp;
}

HolidayContext *holidays_holiday_context_new(const gchar *filename)
{
  return NULL;
}

void holidays_holiday_context_free(HolidayContext *context)
{
}

GList *holidays_get_holidays_for_year(HolidayContext *context,
                                      guint year)
{
  GDate *date = g_date_new_dmy(1, 1, year);
  GList *holidays = NULL;

  holidays_get_fourth_advent(date);
  holidays = holidays_holiday_list_insert(holidays, date, "4. Advent");
  holidays = holidays_holiday_list_insert_relative(holidays, date, -7, "3. Advent");
  holidays = holidays_holiday_list_insert_relative(holidays, date, -14, "2. Advent");
  holidays = holidays_holiday_list_insert_relative(holidays, date, -21, "1. Advent");
  holidays = holidays_holiday_list_insert_relative(holidays, date, -32, "Buß- und Bettag");
  holidays = holidays_holiday_list_insert_relative(holidays, date, -28, "Totensonntag");

  holidays_get_easter_sunday(date);
  holidays = holidays_holiday_list_insert_relative(holidays, date, 60, "Fronleichnam");
  holidays = holidays_holiday_list_insert_relative(holidays, date, 50, "Pfingstmontag");
  holidays = holidays_holiday_list_insert_relative(holidays, date, 49, "Pfingstsonntag");
  holidays = holidays_holiday_list_insert_relative(holidays, date, 39, "Himmelfahrt");
  holidays = holidays_holiday_list_insert_relative(holidays, date, 1, "Ostermontag");
  holidays = holidays_holiday_list_insert(holidays, date, "Ostersonntag");
  holidays = holidays_holiday_list_insert_relative(holidays, date, -1, "Karsamstag");
  holidays = holidays_holiday_list_insert_relative(holidays, date, -2, "Karfreitag");
  holidays = holidays_holiday_list_insert_relative(holidays, date, -3, "Gründonnerstag");

  holidays_get_begin_dst(date);
  holidays = holidays_holiday_list_insert(holidays, date, "Beginn Sommerzeit");

  holidays_get_end_dst(date);
  holidays = holidays_holiday_list_insert(holidays, date, "Ende Sommerzeit");

  holidays = holidays_holiday_list_insert_dmy(holidays,  1,  1, year, "Neujahr");
  holidays = holidays_holiday_list_insert_dmy(holidays,  6,  1, year, "Heilige drei Könige");
  holidays = holidays_holiday_list_insert_dmy(holidays,  1,  5, year, "Maifeiertag");
  holidays = holidays_holiday_list_insert_dmy(holidays,  3, 10, year, "Tag der deutschen Einheit");
  holidays = holidays_holiday_list_insert_dmy(holidays, 24, 12, year, "Heiligabend");
  holidays = holidays_holiday_list_insert_dmy(holidays, 25, 12, year, "1. Weihnachtstag");
  holidays = holidays_holiday_list_insert_dmy(holidays, 26, 12, year, "2. Weihnachtstag");
  holidays = holidays_holiday_list_insert_dmy(holidays, 31, 12, year, "Silvester");
  holidays = holidays_holiday_list_insert_dmy(holidays, 21, 12, year, "Winteranfang");
  holidays = holidays_holiday_list_insert_dmy(holidays, 20,  3, year, "Frühlingsanfang");
  holidays = holidays_holiday_list_insert_dmy(holidays, 21,  6, year, "Sommeranfang");
  holidays = holidays_holiday_list_insert_dmy(holidays, 22,  9, year, "Herbstanfang");
  holidays = holidays_holiday_list_insert_dmy(holidays,  1, 11, year, "Allerheiligen");
  holidays = holidays_holiday_list_insert_dmy(holidays, 31, 10, year, "Reformationstag");
  
  g_date_free(date);
  return holidays;
}

void holidays_holiday_list_free(GList *holiday_list)
{
  g_list_free_full(holiday_list,
                   (GDestroyNotify)holidays_holiday_free);
}
