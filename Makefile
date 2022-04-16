CC ?= gcc
debug ?= no

CFLAGS := -std=gnu99 -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif

all: _all

REAP_DIR := .
include make.mk

.PHONY: all _all install uninstall clean $(CLEAN_TARGETS)

_all: $(REAP_SHARED_LIBRARY) $(REAP_STATIC_LIBRARY)

install: /usr/local/lib/$(notdir $(REAP_SHARED_LIBRARY)) $(foreach file,$(REAP_HEADER_FILES),/usr/local/include/reap/$(notdir $(file)))

/usr/local/lib/$(notdir $(REAP_SHARED_LIBRARY)): $(REAP_SHARED_LIBRARY)
	cp $< $@

/usr/local/include/reap/%.h: $(REAP_INCLUDE_DIR)/reap/%.h /usr/local/include/reap
	cp $< $@

/usr/local/include/reap:
	mkdir -p $@

uninstall:
	rm -rf /usr/local/include/reap
	rm -f /usr/local/lib/$(notdir $(REAP_SHARED_LIBRARY))

clean: $(CLEAN_TARGETS)
	@rm -f $(TESTS) $(DEPS_FILES)