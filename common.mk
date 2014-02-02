CC := gcc
PKG_CONFIG ?= pkg-config
INSTALL ?= install

CFLAGS += -Wall -O2 `$(PKG_CONFIG) --cflags glib-2.0 gtk+-3.0`
LIBS += `$(PKG_CONFIG) --libs glib-2.0 gtk+-3.0`

PREFIX ?= /usr
DATADIR ?= $(PREFIX)/share/gcalendar

gcal_SRC := $(filter-out gcal-remote.c, $(wildcard *.c))
gcal_OBJ := $(gcal_SRC:.c=.o)
gcal_HEADERS := $(wildcard *.h)

gcal_rem_SRC := gcal-remote.c
gcal_rem_OBJ := $(gcal_rem_SRC:.c=.o)
gcal_rem_HEADERS := 

CFLAGS += -DGCALENDARDATADIR=\"${DATADIR}\"
