TEST_SOURCE_FILES := $(wildcard $(TEST_DIR)/*.c)
TEST_BINARY := $(TEST_DIR)/tests

.PHONY: tests

tests: $(TEST_BINARY)
	@echo
	@$<

$(TEST_BINARY): $(TEST_SOURCE_FILES) $(REAP_SHARED_LIBRARY)
	$(CC) $(CFLAGS) $(REAP_INCLUDE_FLAGS) $(filter %.c,$^) -Wl,-rpath $(realpath $(REAP_LIB_DIR)) -L$(REAP_LIB_DIR) -lreap -lscrutiny -pthread -o $@

test_clean:
	@rm -f $(TEST_BINARY)

CLEAN_TARGETS += test_clean
