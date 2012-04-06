#include <gtk/gtk.h>
#include "holidays.h"
#include "calendar.h"

struct Configuration {
  gboolean attach_to_tray;
  gchar *holiday_file;
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
  gtk_widget_size_request(window, &win_req);

  if (orientation == GTK_ORIENTATION_VERTICAL) {
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

static GOptionEntry main_option_entries[] = {
  { "tray", 't', 0, G_OPTION_ARG_NONE, &config.attach_to_tray,
    "Attach to tray", "value" },
  { "holiday-file", 'f', 0, G_OPTION_ARG_STRING, &config.holiday_file,
    "Holiday file", "value" },
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

  if (config.attach_to_tray) {
    window = gtk_window_new(GTK_WINDOW_POPUP);

    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
  }
  else {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
  }

  gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);

  gtk_container_add(GTK_CONTAINER(window), calendar_widget);

  return window;
}

void main_set_up_application(GtkWidget *window)
{
  if (!config.attach_to_tray) {
    gtk_widget_show_all(window);
    return;
  }

  status_icon = gtk_status_icon_new_from_stock(GTK_STOCK_ABOUT);
  gtk_status_icon_set_visible(status_icon, TRUE);

  g_signal_connect(G_OBJECT(status_icon),
                   "activate",
                   G_CALLBACK(main_tray_icon_clicked),
                   window);
}

int main(int argc, char **argv)
{
  GtkWidget *window;

  gtk_init(&argc, &argv);
  if (!main_read_config(argc, argv)) {
    g_error("Could not read configuration\n");
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
