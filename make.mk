REAP_LIB_DIR ?= $(REAP_DIR)
REAP_OBJ_DIR ?= $(REAP_DIR)/src

REAP_SHARED_LIBRARY := $(REAP_LIB_DIR)/libreap.so
REAP_STATIC_LIBRARY := $(REAP_LIB_DIR)/libreap.a

REAP_SOURCE_FILES := $(wildcard $(REAP_DIR)/src/*.c)
REAP_OBJECT_FILES := $(patsubst $(REAP_DIR)/src/%.c,$(REAP_OBJ_DIR)/%.o,$(REAP_SOURCE_FILES))
REAP_HEADER_FILES := $(wildcard $(REAP_DIR)/include/reap/*.h)
REAP_INCLUDE_FLAGS := -I$(REAP_DIR)/include

REAP_DEPS_FILE := $(REAP_DIR)/deps.mk
DEPS_FILES += $(REAP_DEPS_FILE)

BUILD_DEPS ?= $(if $(MAKECMDGOALS),$(subst clean,,$(MAKECMDGOALS)),yes)

ifneq ($(BUILD_DEPS),)

$(REAP_DEPS_FILE): $(REAP_SOURCE_FILES) $(REAP_HEADER_FILES)
	rm -f $@
	for file in $(REAP_SOURCE_FILES); do \
	    echo "$(REAP_OBJ_DIR)/`$(CC) $(REAP_INCLUDE_FLAGS) -MM $$file`" >> $@ && \
	    echo '\t$$(CC) $$(CFLAGS) -fpic -ffunction-sections $(REAP_INCLUDE_FLAGS) -c $$< -o $$@' >> $@; \
	done
include $(REAP_DEPS_FILE)

endif

$(REAP_SHARED_LIBRARY): $(REAP_OBJECT_FILES)
	$(CC) $(LDFLAGS) -shared -o $@ $^

$(REAP_STATIC_LIBRARY): $(REAP_OBJECT_FILES)
	$(AR) rcs $@ $^

reap_clean:
	@rm -f $(REAP_SHARED_LIBRARY) $(REAP_STATIC_LIBRARY) $(REAP_OBJECT_FILES)

CLEAN_TARGETS += reap_clean
