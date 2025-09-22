PROJECT_NAME = alterator-backend-packages
LINGUAS = $(shell cat LINGUAS 2>/dev/null)

.PHONY: all compile install clean

all: compile

compile:
	@for lang in $(LINGUAS); do \
		echo "Compiling $$lang translation..."; \
		mkdir -p $$lang/LC_MESSAGES; \
		[ -f $$lang.po ] && msgfmt -o $$lang/LC_MESSAGES/$(PROJECT_NAME).mo $$lang.po || echo "Warning: $$lang.po not found"; \
	done

install: DESTDIR ?= /
install: DATADIR = $(DESTDIR)/usr/share
install: LOCALE_DIR = $(DATADIR)/locale
install: compile
	@for lang in $(LINGUAS); do \
		if [ -f $$lang/LC_MESSAGES/$(PROJECT_NAME).mo ]; then \
			echo "Installing $$lang translation..."; \
			mkdir -p $(LOCALE_DIR)/$$lang/LC_MESSAGES; \
			install -v -p -m 644 $$lang/LC_MESSAGES/$(PROJECT_NAME).mo $(LOCALE_DIR)/$$lang/LC_MESSAGES/; \
		else \
			echo "Warning: $$lang/LC_MESSAGES/$(PROJECT_NAME).mo not found - skipping"; \
		fi; \
	done

clean:
	rm -rf */LC_MESSAGES