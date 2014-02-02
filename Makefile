include common.mk

all: gcalendar gcal-remote

gcalendar gcal-remote:
	$(MAKE) -C src $@

clean:
	$(MAKE) -C src clean

install: gcalendar gcal-remote install-data
	$(MAKE) -C src install

install-data:
	$(MAKE) -C data install-data

.PHONY: all clean install install-data
