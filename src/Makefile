include ../common.mk

gcalendar: $(gcal_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

gcal-remote: $(gcal_rem_OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

%.o: %.c $(gcal_HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	$(RM) -f gcal-remote gcalendar $(gcal_OBJ) $(gcal_rem_OBJ)

install: gcalendar gcal-remote
	$(INSTALL) gcalendar $(PREFIX)/bin
	$(INSTALL) gcal-remote $(PREFIX)/bin

.PHONY: clean install
