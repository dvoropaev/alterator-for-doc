.PHONY: install

PROJECT_NAME=alterator-backend-packages

DESTDIR?=/
DATADIR=$(DESTDIR)/usr/share
ALTERATOR_DATADIR=$(DATADIR)/alterator
ALTERATOR_BACKENDS_DATADIR=$(ALTERATOR_DATADIR)/backends
ALTERATOR_OBJECTS_DATADIR=$(ALTERATOR_DATADIR)/objects
ALTERATOR_LOGDIR=$(DESTDIR)/var/log/alterator
LIBDIR=$(DESTDIR)/usr/lib
INTERFACES_DATADIR=$(DATADIR)/dbus-1/interfaces
ACTIONS_DATADIR=$(DATADIR)/polkit-1/actions
SYS_CONFDIR=$(DESTDIR)/etc
APT_CONFDIR=$(SYS_CONFDIR)/apt/apt.conf.d
LOGROTATE_CONFDIR=$(SYS_CONFDIR)/logrotate.d/
APT_DATADIR=$(DATADIR)/apt

install_interface_prepare:
	mkdir -p $(DATADIR)/dbus-1/interfaces
	mkdir -p $(DATADIR)/polkit-1/actions

install_interface_apt:
	install -v -p -m 644 -D apt/*.xml $(INTERFACES_DATADIR)/
	install -v -p -m 644 -D apt/*.policy $(ACTIONS_DATADIR)/

install_interface_rpm:
	install -v -p -m 644 -D rpm/*.xml $(INTERFACES_DATADIR)/
	install -v -p -m 644 -D rpm/*.policy $(ACTIONS_DATADIR)/

install_interface_repo:
	install -v -p -m 644 -D repo/*.xml $(INTERFACES_DATADIR)/
	install -v -p -m 644 -D repo/*.policy $(ACTIONS_DATADIR)/

install_backend_prepare:
	mkdir -p $(LIBDIR)/$(PROJECT_NAME)
	mkdir -p $(ALTERATOR_BACKENDS_DATADIR)
	mkdir -p $(ALTERATOR_OBJECTS_DATADIR)

install_backend_apt:
	mkdir -p $(ALTERATOR_LOGDIR)/apt
	mkdir -p $(APT_DATADIR)/scripts
	mkdir -p $(APT_CONFDIR)
	mkdir -p $(LOGROTATE_CONFDIR)
	install -v -p -m 644 -D apt/*.backend $(ALTERATOR_BACKENDS_DATADIR)/
	install -v -p -m 644 -D apt/*.object $(ALTERATOR_OBJECTS_DATADIR)/
	install -v -p -m 755 -D apt/apt-wrapper $(LIBDIR)/$(PROJECT_NAME)/
	install -v -p -m 755 -D apt/logger/*.lua $(APT_DATADIR)/scripts/
	install -v -p -m 755 -D apt/logger/*.conf $(APT_CONFDIR)/
	install -v -p -m 755 -D apt/logger/*.logrotate $(LOGROTATE_CONFDIR)/
	touch $(ALTERATOR_LOGDIR)/apt/dist-upgrades.log
	chmod 644 $(ALTERATOR_LOGDIR)/apt/dist-upgrades.log

install_backend_rpm:
	install -v -p -m 644 -D rpm/*.backend $(ALTERATOR_BACKENDS_DATADIR)/
	install -v -p -m 644 -D rpm/*.object $(ALTERATOR_OBJECTS_DATADIR)/
	install -v -p -m 755 -D rpm/rpm-wrapper $(LIBDIR)/$(PROJECT_NAME)/

install_backend_repo:
	install -v -p -m 644 -D repo/*.backend $(ALTERATOR_BACKENDS_DATADIR)/
	install -v -p -m 644 -D repo/*.object $(ALTERATOR_OBJECTS_DATADIR)/

install_interface: \
	install_interface_prepare \
	install_interface_apt \
	install_interface_rpm \
	install_interface_repo

install_backend: \
	install_backend_prepare \
	install_backend_apt \
	install_backend_rpm \
	install_backend_repo

install: \
	install_interface \
	install_backend
	@$(MAKE) -C po install DESTDIR=$(DESTDIR) || echo "Warning: Failed to install translations"
