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

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "holidays.h"
#include "calendar.h"
#include "config.h"

#define MAIN_MENU_ITEM_TOGGLE_DISPLAY_HOLIDAYS     1
#define MAIN_MENU_ITEM_TOGGLE_DISPLAY_TASKS        2

struct Configuration {
  gboolean attach_to_tray;
  gchar *holiday_file;
  gchar *config_file;
  gboolean show_holidays;
  gboolean show_tasks;
} config;

CalendarWidget *calendar = NULL;
HolidayContext *holiday_context = NULL;
GtkStatusIcon *status_icon = NULL;
gboolean tray_window_visible = FALSE;

void main_tray_position_window(GtkWidget *window)
{
  GdkRectangle area;
  GtkOrientation orientation;
  GdkScreen *screen;
  gint x;
  gint y;
  gint monitor_num;
  GdkRectangle monitor;
  GtkRequisition win_req;

  g_return_if_fail(GTK_IS_WINDOW(window));

  if (!status_icon) {
    return;
  }

  if (!gtk_status_icon_get_geometry(status_icon,
                                    &screen,
                                    &area,
                                    &orientation)) {
    g_warning("Unable to determine geometry of status icon\n");
    return;
  }

  gtk_window_set_screen(GTK_WINDOW(window), screen);

  monitor_num = gdk_screen_get_monitor_at_point(screen, area.x, area.y);
  gdk_screen_get_monitor_geometry(screen, monitor_num, &monitor);
  
  gtk_container_foreach(GTK_CONTAINER(window), (GtkCallback)gtk_widget_show_all, NULL);
#ifdef USE_GTK2
  gtk_widget_size_request(window, &win_req);
#else
  gtk_widget_get_preferred_size(window, &win_req, NULL);
#endif

  if (orientation == GTK_ORIENTATION_VERTICAL) {
    if (area.x + area.width + win_req.width <= monitor.x + monitor.width) {
      x = area.x + area.width;
    }
    else {
      x = area.x - win_req.width;
    }
    if (area.y + win_req.height <= monitor.y + monitor.height) {
      y = area.y;
    }
    else {
      y = monitor.y + monitor.height - win_req.height;
    }
  }
  else {
    if (area.y + area.height + win_req.height <= monitor.y + monitor.height) {
      y = area.y + area.height;
    }
    else {
      y = area.y - win_req.height;
    }
    if (area.x + win_req.width <= monitor.x + monitor.width) {
      x = area.x;
    }
    else {
      x = monitor.x + monitor.width - win_req.width;
    }
  }

  gtk_window_move(GTK_WINDOW(window), x, y);
}

void main_tray_icon_set_visible(GtkWidget *tray_widget,
                                gboolean visible)
{
  tray_window_visible = visible;

  if (visible) {
    main_tray_position_window(tray_widget);
    gtk_widget_show_all(tray_widget);
  }
  else {
    gtk_widget_hide(tray_widget);
  }
}

void main_tray_icon_toggle_display(GtkWidget *tray_widget)
{
  main_tray_icon_set_visible(tray_widget, !tray_window_visible);
}

void main_tray_icon_clicked(GtkStatusIcon *icon,
                            GtkWidget *tray_widget)
{
  main_tray_icon_toggle_display(tray_widget);
}

void main_menu_toggle_display(GtkCheckMenuItem *item,
                              gpointer user_data)
{
  gint id = GPOINTER_TO_INT(user_data);
  if (id == MAIN_MENU_ITEM_TOGGLE_DISPLAY_HOLIDAYS) {
    if (gtk_check_menu_item_get_active(item)) {
      g_print("holidays active\n");
    }
    else {
      g_print("holidays not active\n");
    }
  }
}

GtkWidget *main_create_main_menu(GtkAccelGroup *accel)
{
  GtkWidget *item; 
  GtkWidget *menu;
 
  menu = gtk_menu_new();

  item = gtk_check_menu_item_new_with_label("Show holidays");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), config.show_holidays);
  g_signal_connect(G_OBJECT(item),
                   "toggled",
                   G_CALLBACK(main_menu_toggle_display),
                   GINT_TO_POINTER(MAIN_MENU_ITEM_TOGGLE_DISPLAY_HOLIDAYS));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  if (accel) {
    gtk_widget_add_accelerator(item, "activate", accel, GDK_KEY_h, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  }
  
  item = gtk_check_menu_item_new_with_label("Show tasks");
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), config.show_tasks);
  g_signal_connect(G_OBJECT(item),
                   "toggled",
                   G_CALLBACK(main_menu_toggle_display),
                   GINT_TO_POINTER(MAIN_MENU_ITEM_TOGGLE_DISPLAY_TASKS));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  if (accel) {
    gtk_widget_add_accelerator(item, "activate", accel, GDK_KEY_t, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  }

  gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());

  item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
  g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(gtk_main_quit), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
  if (accel) {
    gtk_widget_add_accelerator(item, "activate", accel, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  }

  return menu;
}

void main_tray_icon_menu(GtkStatusIcon *icon,
                         guint button,
                         guint activate_time,
                         gpointer user_data)
{
  GtkWidget *popup_menu = main_create_main_menu(NULL);
  gtk_widget_show_all(popup_menu);

  gtk_menu_popup(GTK_MENU(popup_menu),
                 NULL,
                 NULL, 
                 (GtkMenuPositionFunc)gtk_status_icon_position_menu,
                 (gpointer)icon,
                 button,
                 activate_time);
}

gboolean main_read_configuration_file(void)
{
  GKeyFile *file = NULL;
  const gchar *config_dirs[2];

  if (config.holiday_file) {
    return TRUE;
  }

  file = g_key_file_new();

  if (config.config_file) {
    if (!g_key_file_load_from_file(file, config.config_file, 0, NULL)) {
      g_key_file_free(file);
      return FALSE;
    }
  }
  else {
    config_dirs[0] = g_get_user_config_dir();
    config_dirs[1] = NULL;

    if (!g_key_file_load_from_dirs(file, "gcalendarrc", config_dirs, NULL, 0, NULL) &&
        !g_key_file_load_from_dirs(file, "gcalendarrc", (const gchar**)g_get_system_config_dirs(), NULL, 0, NULL)) {
      g_key_file_free(file);
      return FALSE;
    }
  }

  config.holiday_file = g_key_file_get_string(file, "Holidays", "source", NULL);

  g_key_file_free(file);

  return TRUE;
}

static GOptionEntry main_option_entries[] = {
  { "tray", 't', 0, G_OPTION_ARG_NONE, &config.attach_to_tray,
    "Attach to tray", "value" },
  { "holiday-file", 'f', 0, G_OPTION_ARG_STRING, &config.holiday_file,
    "Holiday file", "value" },
  { "config", 'c', 0, G_OPTION_ARG_STRING, &config.config_file,
    "Configuration file", "value" },
  NULL
};

gboolean main_read_config(int argc, char **argv)
{
  GOptionContext *context = g_option_context_new(" - calendar tool that displays holidays");

  g_option_context_add_main_entries(context, main_option_entries, NULL);
/*  g_option_context_add_group(context, gtk_get_option_group(TRUE));*/

  if (!g_option_context_parse(context, &argc, &argv, NULL)) {
    g_option_context_free(context);
    return FALSE;
  }

  g_option_context_free(context);

  return TRUE;
}

GtkWidget *main_create_window(GtkWidget *calendar_widget)
{
  GtkWidget *window;
  GtkWidget *menubar, *item, *submenu, *grid;
  GtkAccelGroup *accel = NULL;

  if (config.attach_to_tray) {
    window = gtk_window_new(GTK_WINDOW_POPUP);

    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);

    gtk_container_add(GTK_CONTAINER(window), calendar_widget);
  }
  else {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);
  
    accel = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel);

    menubar = gtk_menu_bar_new();
    item = gtk_menu_item_new_with_label("Calendar");

    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), item);

    submenu = main_create_main_menu(accel);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);

#ifdef USE_GTK2
    grid = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(grid), menubar, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(grid), calendar_widget, TRUE, TRUE, 0);
#else
    grid = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), FALSE);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(grid), GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(grid), menubar);
    gtk_container_add(GTK_CONTAINER(grid), calendar_widget);
#endif

    gtk_widget_show_all(grid);

    gtk_container_add(GTK_CONTAINER(window), grid);
  }

  return window;
}

void main_set_up_application(GtkWidget *window)
{
  if (!config.attach_to_tray) {
    gtk_widget_show_all(window);
    return;
  }

  g_print("Try to load file %s\n", GCALENDARDATADIR "/gcalendar.svg");

  status_icon = gtk_status_icon_new_from_file(GCALENDARDATADIR "/gcalendar.svg");
  gtk_status_icon_set_visible(status_icon, TRUE);

  g_signal_connect(G_OBJECT(status_icon),
                   "activate",
                   G_CALLBACK(main_tray_icon_clicked),
                   window);
  g_signal_connect(G_OBJECT(status_icon),
                   "popup-menu",
                   G_CALLBACK(main_tray_icon_menu),
                   window);
}

int main(int argc, char **argv)
{
  GtkWidget *window;

  gtk_init(&argc, &argv);
  if (!main_read_config(argc, argv)) {
    g_warning("Could not read command line\n");
    return 1;
  }

  if (!main_read_configuration_file()) {
    g_warning("Could not read configuration file\n");
    return 1;
  }

  holiday_context = holidays_holiday_context_new(config.holiday_file);

  calendar = calendar_widget_new();
  calendar_widget_set_holiday_context(calendar, holiday_context);

  window = main_create_window(calendar->widget);
  main_set_up_application(window);

  gtk_main();

  return 0;
}
