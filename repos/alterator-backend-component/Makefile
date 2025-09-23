.PHONY: install

SHORT_NAME=component
PROJECT_NAME=alterator-backend-$(SHORT_NAME)

DESTDIR?=/
DATADIR=$(DESTDIR)/usr/share
ALTERATOR_DATADIR=$(DATADIR)/alterator
ALTERATOR_LIBDIR=$(DESTDIR)/usr/lib/alterator

install_interface:
	mkdir -p $(DATADIR)/dbus-1/interfaces
	mkdir -p $(DATADIR)/polkit-1/actions
	install -v -p -m 644 -D interface/org.altlinux.alterator.*.xml $(DATADIR)/dbus-1/interfaces/
	install -v -p -m 644 -D interface/org.altlinux.alterator.*.policy $(DATADIR)/polkit-1/actions/

install_backend:
	mkdir -p $(ALTERATOR_DATADIR)/backends
	install -v -p -m 644 -D backend/*.backend $(ALTERATOR_DATADIR)/backends
	mkdir -p $(ALTERATOR_LIBDIR)/backends/$(SHORT_NAME).d
	install -v -p -m 755 -D backend/$(SHORT_NAME) $(ALTERATOR_LIBDIR)/backends/
	install -v -p -m 755 -D backend/$(SHORT_NAME).d/* $(ALTERATOR_LIBDIR)/backends/$(SHORT_NAME).d

install: \
	install_interface \
	install_backend \
