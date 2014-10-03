/*
 *  gcalendar -- a calendar tray widget
 *  Copyright (C) 2012-2014 Holger Langenau (see also: LICENSE)
 *
 */
#include "ipc-server.h"
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "commands.h"

int server_socket = -1;
GList *client_list = NULL;
GThread *server_thread = NULL;
int server_ctrl_pipe[] = { -1, -1 };

union {
    struct sockaddr sa;
    struct sockaddr_un un;
} addr;

struct IPCCommand {
    gchar *cmd;
    void (*callback)(void);
};

int _ipc_server_init(char *address);
void _ipc_server_accept(void);
void _ipc_server_remove_client(int client_fd);
void _ipc_server_serve(int client_fd);
int _ipc_server_thread_proc(gpointer data);
void _ipc_server_run_command(char *buffer);

int ipc_server_start(char *address)
{
    if (_ipc_server_init(address) != 0)
        return 1;

    if (pipe(server_ctrl_pipe) != 0) {
        fprintf(stderr, "Could not create conrol pipe.\n");
        return 1;
    }

#if GLIB_CHECK_VERSION(2, 32, 0) /* g_thread_new since 2.32 */
    server_thread = g_thread_new("gcalserver", (GThreadFunc)_ipc_server_thread_proc, NULL);
#else
    server_thread = g_thread_create((GThreadFunc)_ipc_server_thread_proc,
                                    NULL, TRUE, NULL);
#endif

    if (server_thread == NULL) {
        fprintf(stderr, "Could not create server thread.\n");
        return 1;
    }

    return 0;
}

void ipc_server_stop(void)
{
    /* send quit signal to pipe */
    if (server_ctrl_pipe[1] != -1) {
        write(server_ctrl_pipe[1], "quit", 4);
        /* wait for thread */
        g_thread_join(server_thread);

        close(server_ctrl_pipe[0]);
        close(server_ctrl_pipe[1]);

        server_ctrl_pipe[0] = -1;
        server_ctrl_pipe[1] = -1;
        server_thread = NULL;
    }
    close(server_socket);
    unlink(addr.un.sun_path);
}

int _ipc_server_init(char *address)
{
    int sock;

    if (!address || address[0] == '\0') {
        fprintf(stderr, "Empty address given.\n");
        return 1;
    }

    addr.sa.sa_family = AF_UNIX;
    strncpy(addr.un.sun_path, address, sizeof(addr.un.sun_path) - 1);

    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket == -1) {
        fprintf(stderr, "Could not create server socket.\n");
        return 1;
    }

    if (bind(server_socket, &addr.sa, sizeof(struct sockaddr_un)) == -1) {
        if (errno != EADDRINUSE) {
            fprintf(stderr, "Could not bind server socket.\n");
            return 1;
        }
        fprintf(stderr, "Address is already in use. Checking if active.\n");

        /* try to connect to socket */
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock == -1) {
            fprintf(stderr, "Could not create socket.\n");
            return 1;
        }

        if (connect(sock, &addr.sa, sizeof(struct sockaddr_un)) == -1) {
            if (errno != ENOENT && errno != ECONNREFUSED) {
                fprintf(stderr, "Could not connect to existing socket.\n");
                return 1;
            }

            /* server not running: dead socket */
            /* try to remove dead socket */
            if (unlink(addr.un.sun_path) == -1 && errno != ENOENT) {
                fprintf(stderr, "Could not remove dead socket.\n");
                return 1;
            }
            if (bind(server_socket, &addr.sa, sizeof(struct sockaddr_un)) == -1) {
                fprintf(stderr, "Could not bint server socket.\n");
                return 1;
            }
        }
        else {
            fprintf(stderr, "Someone is already listening on socket %s.\n", address);
            return 1;
        }
        close(sock);
    }

    if (listen(server_socket, 10) == -1) {
        fprintf(stderr, "Could not listen on server socket.\n");
        return 1;
    }

    return 0;
}

void _ipc_server_accept(void)
{
    struct sockaddr sa;
    socklen_t sa_size = sizeof(sa);
    int fd;

    fd = accept(server_socket, &sa, &sa_size);
    if (fd == -1) {
        return;
    }

    fcntl(fd, F_SETFL, O_NONBLOCK);

    client_list = g_list_append(client_list, GINT_TO_POINTER(fd));
}

void _ipc_server_remove_client(int client_fd)
{
    close(client_fd);
    client_list = g_list_remove(client_list, GINT_TO_POINTER(client_fd));
}

void _ipc_server_serve(int client_fd)
{
    char buf[1024];
    int rc;

    do {
        rc = read(client_fd, buf, sizeof(buf) - 1);
        if (rc == -1) {
            if (errno == EINTR) {
                continue;
            }
            if (errno == EAGAIN) {
                return;
            }
            _ipc_server_remove_client(client_fd);
        }
        if (rc == 0) {
            _ipc_server_remove_client(client_fd);
            return;
        }
    }
    while (0);

    buf[rc] = '\0';

    _ipc_server_run_command(buf);
}

int _ipc_server_thread_proc(gpointer data)
{
    fd_set set;
    int max;
    GList *cur;

    while (1) {
        FD_ZERO(&set);
        FD_SET(server_socket, &set);
        FD_SET(server_ctrl_pipe[0], &set);
        max = server_socket > server_ctrl_pipe[0] ? server_socket : server_ctrl_pipe[0];

        for (cur = client_list; cur != NULL; cur = cur->next) {
            FD_SET(GPOINTER_TO_INT(cur->data), &set);
            if (GPOINTER_TO_INT(cur->data) > max)
                max = GPOINTER_TO_INT(cur->data);
        }

        if (select(max+1, &set, NULL, NULL, NULL) < 0) {
            fprintf(stderr, "Lost connection.\n");
            return 1;
        }

        if (FD_ISSET(server_socket, &set)) {
            _ipc_server_accept();
        }

        for (cur = client_list; cur != NULL; cur = cur->next) {
            if (FD_ISSET(GPOINTER_TO_INT(cur->data), &set)) {
                _ipc_server_serve(GPOINTER_TO_INT(cur->data));
            }
        }

        /* check pipe */
        if (FD_ISSET(server_ctrl_pipe[0], &set)) {
            fprintf(stderr, "Pipe set: stop.\n");
            return 0;
        }
    }
    return 0;
}

gboolean _ipc_server_set_idle_command(void (*func)(void))
{
    if (func) {
        func();
    }
    else {
        fprintf(stderr, "No callback for this command set.\n");
    }
    /* remove source */
    return FALSE;
}

static struct IPCCommand _ipc_commands[] = {
    { "toggle display", command_toggle_display },
    { "select today",   command_select_today },
    { "next month",     command_next_month },
    { "prev month",     command_prev_month },
    { "next year",      command_next_year },
    { "prev year",      command_prev_year },
    { NULL, NULL}
};

void _ipc_server_run_command(char *buffer)
{
    int i;

    for (i = 0; _ipc_commands[i].cmd != NULL; ++i) {
        if (strcmp(buffer, _ipc_commands[i].cmd) == 0) {
            g_idle_add((GSourceFunc)_ipc_server_set_idle_command,
                       (gpointer)_ipc_commands[i].callback);
            return;
        }
    }

    fprintf(stderr, "Unknown command: %s\n", buffer);
}
