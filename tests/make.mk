TEST_BINARIES := $(patsubst $(TEST_DIR)/test_%.c,$(TEST_DIR)/test_binary_%,$(wildcard $(TEST_DIR)/test_*.c))

$(TEST_DIR)/test_binary_%: $(TEST_DIR)/test_%.c $(REAP_STATIC_LIBRARY)
	$(CC) $(CFLAGS) $(REAP_INCLUDE_FLAGS) -o $@ $^

TEST_IMAGE := reap_testing

tests: $(TEST_BINARIES)

test_clean:
	@rm -f $(TEST_BINARIES)

CLEAN_TARGETS += test_clean
