CFLAGS := -std=gnu11 -fdiagnostics-color -Wall -Wextra
ifeq ($(debug),yes)
    CFLAGS += -O0 -g -DDEBUG
else
    CFLAGS += -O2 -DNDEBUG
endif

all: _all

BUILD_DEPS :=
ifeq ($(MAKECMDGOALS),clean)
else ifeq ($(MAKECMDGOALS),format)
else ifeq ($(MAKECMDGOALS),uninstall)
else
    BUILD_DEPS := yes
endif

REAP_DIR := .
include make.mk

TEST_DIR := tests
include $(TEST_DIR)/make.mk

.PHONY: all _all tests format install uninstall clean $(CLEAN_TARGETS)

_all: $(REAP_SHARED_LIBRARY) $(REAP_STATIC_LIBRARY)

install: /usr/local/lib/$(notdir $(REAP_SHARED_LIBRARY)) $(foreach file,$(REAP_HEADER_FILES),/usr/local/include/reap/$(notdir $(file)))

/usr/local/lib/$(notdir $(REAP_SHARED_LIBRARY)): $(REAP_SHARED_LIBRARY)
	cp $< $@

/usr/local/include/reap/%.h: include/reap/%.h /usr/local/include/reap
	cp $< $@

/usr/local/include/reap:
	mkdir -p $@

uninstall:
	rm -rf /usr/local/include/reap
	rm -f /usr/local/lib/$(notdir $(REAP_SHARED_LIBRARY))

format:
	find . -name '*.[hc]' -print0 | xargs -0 -n 1 clang-format -i

clean: $(CLEAN_TARGETS)
	@rm -f $(DEPS_FILES)
