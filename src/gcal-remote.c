#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <glib.h>

int sock_fd = -1;

static union {
  struct sockaddr sa;
  struct sockaddr_un un;
} addr;

struct {
  char *socket_path;
  gboolean cmd_toggle_display;
  gboolean cmd_today;
  gboolean cmd_next_month;
  gboolean cmd_prev_month;
  gboolean cmd_next_year;
  gboolean cmd_prev_year;
} cmd;

int remote_connect(char *address);
ssize_t write_all(int fd, char *buf, size_t count);
gboolean parse_options(int argc, char **argv);

int main(int argc, char **argv) {
  if (!parse_options(argc, argv)) {
    fprintf(stderr, "Error parsing command line.\n");
    return 1;
  }
  if (remote_connect(cmd.socket_path) != 0)
    return 1;

  if (cmd.cmd_toggle_display)
    write_all(sock_fd, "toggle display", 14);

  if (cmd.cmd_today)
    write_all(sock_fd, "select today", 12);

  if (cmd.cmd_next_month)
    write_all(sock_fd, "next month", 10);

  if (cmd.cmd_prev_month)
    write_all(sock_fd, "prev month", 10);

  if (cmd.cmd_next_year)
    write_all(sock_fd, "next year", 9);

  if (cmd.cmd_prev_year)
    write_all(sock_fd, "prev year", 9);

  close(sock_fd);

  return 0;
}

int remote_connect(char *address)
{
  if (address == NULL || address[0] == '\0') {
    fprintf(stderr, "No address given.\n");
    return 1;
  }
  addr.sa.sa_family = AF_UNIX;
  strncpy(addr.un.sun_path, address, sizeof(addr.un.sun_path) - 1);

  sock_fd = socket(addr.sa.sa_family, SOCK_STREAM, 0);
  if (sock_fd == -1) {
    fprintf(stderr, "Could not create client socket.\n");
    return 1;
  }

  if (connect(sock_fd, &addr.sa, sizeof(addr.un)) != 0) {
    fprintf(stderr, "Could not connect to socket.\n");
    return 1;
  }

  return 0;
}

ssize_t write_all(int fd, char *buf, size_t count)
{
  const char *buffer = buf;
  size_t cs = count;
  int rc;

  do {
    rc = write(fd, buffer, count);
    if (rc == -1) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      return -1;
    }
    buffer += rc;
    count -= rc;
  } while (count > 0);
  return cs;
}

static GOptionEntry main_option_entries[] = {
  { "socket-path", 's', 0, G_OPTION_ARG_STRING, &cmd.socket_path,
    "Socket path for IPC", "value" },
  { "toggle-display", 't', 0, G_OPTION_ARG_NONE, &cmd.cmd_toggle_display,
    "Toggle display of tray widget", NULL },
  { "today", 0, 0, G_OPTION_ARG_NONE, &cmd.cmd_today,
    "Select today’s date", NULL },
  { "next-month", 'n', 0, G_OPTION_ARG_NONE, &cmd.cmd_next_month,
    "Select next month", NULL },
  { "prev-month", 'p', 0, G_OPTION_ARG_NONE, &cmd.cmd_prev_month,
    "Select previous month", NULL },
  { "next-year", 0, 0, G_OPTION_ARG_NONE, &cmd.cmd_next_year,
    "Select next year", NULL },
  { "prev-year", 0, 0, G_OPTION_ARG_NONE, &cmd.cmd_prev_year,
    "Select previous year", NULL },
  NULL
};

gboolean parse_options(int argc, char **argv)
{
  GOptionContext *context = g_option_context_new(" - send commands to gcalendar");

  g_option_context_add_main_entries(context, main_option_entries, NULL);

  if (!g_option_context_parse(context, &argc, &argv, NULL)) {
    g_option_context_free(context);
    return FALSE;
  }

  g_option_context_free(context);

  return TRUE;
}
