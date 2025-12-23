# Makefile for Pebble Timer
# Supports building, testing, CloudPebble deployment, and IP-based deployment

.PHONY: all build clean install-cloudpebble install-ip test test-build help

# Compiler settings for tests (native compilation, not Pebble)
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -I tests -I src/c
TEST_SRCS = tests/test_main.c tests/test_time_utils.c tests/test_timer_state.c \
            src/c/time_utils.c src/c/timer_state.c
TEST_BIN = build/tests/test_runner

# Default target
all: build

build:
	pebble build

clean:
	pebble clean
	rm -rf build/

# =============================================================================
# Testing
# =============================================================================

# Build and run tests
test: test-build
	@echo ""
	@./$(TEST_BIN)

# Build tests only
test-build:
	@mkdir -p build/tests
	@echo "Building unit tests..."
	@$(CC) $(CFLAGS) -o $(TEST_BIN) $(TEST_SRCS)
	@echo "Test binary built: $(TEST_BIN)"

# Run tests with verbose output
test-verbose: test-build
	@./$(TEST_BIN) -v

# =============================================================================
# Deployment
# =============================================================================

# Install to CloudPebble
install-cloudpebble: build
	pebble install --cloudpebble

# Install to device via IP address
# Usage: make install-ip IP=192.168.1.100
install-ip: build
	@if [ -z "$(IP)" ]; then \
		echo "Error: IP address not specified. Usage: make install-ip IP=192.168.1.100"; \
		exit 1; \
	fi
	@echo "Installing to device at $(IP)..."
	pebble install --phone $(IP)

# =============================================================================
# Development Helpers
# =============================================================================

# Check code formatting and style
lint:
	@echo "Checking code style..."
	@find src/c -name "*.c" -o -name "*.h" | xargs -I {} sh -c 'echo "Checking {}"; gcc -fsyntax-only -Wall -Wextra {} 2>&1 || true'


help:
	@echo "Pebble Timer Makefile"
	@echo ""
	@echo "Build targets:"
	@echo "  make build               - Build the Pebble app"
	@echo "  make clean               - Clean build artifacts"
	@echo ""
	@echo "Test targets:"
	@echo "  make test                - Build and run unit tests"
	@echo "  make test-build          - Build tests only"
	@echo "  make test-verbose        - Run tests with verbose output"
	@echo ""
	@echo "Deployment targets:"
	@echo "  make install-cloudpebble - Install to CloudPebble"
	@echo "  make install-ip IP=<ip>  - Install to device via IP address"
	@echo ""
	@echo "Development targets:"
	@echo "  make lint                - Check code for warnings"
