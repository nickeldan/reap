TEST_SOURCE_FILES := $(wildcard $(TEST_DIR)/*.c)
TEST_BINARY := $(TEST_DIR)/test

.PHONY: tests

tests: $(TEST_BINARY)
	@echo
	@$<

$(TEST_BINARY): $(TEST_SOURCE_FILES) $(REAP_STATIC_LIBRARY)
	$(CC) $(CFLAGS) $(REAP_INCLUDE_FLAGS) $^ -lscrutiny -pthread -o $@

test_clean:
	@rm -f $(TEST_BINARY)

CLEAN_TARGETS += test_clean
