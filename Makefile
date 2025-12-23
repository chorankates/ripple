# Makefile for Pebble Timer
# Supports building, testing, CloudPebble deployment, and IP-based deployment

.PHONY: all build clean install-cloudpebble install-ip test test-build test-verbose \
        emulator emulator-aplite emulator-basalt emulator-chalk emulator-diorite emulator-emery \
        screenshot screenshot-all emulator-kill screenshot-mode screenshot-all-modes screenshot-matrix \
        lint help

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
# Emulator & Screenshots
# =============================================================================

# Directory for screenshots
SCREENSHOT_DIR = screenshots

# Available platforms (subset - most commonly used)
PLATFORMS = aplite basalt chalk diorite emery

# Display modes (in order, matching DisplayMode enum in timer_state.h)
# Index corresponds to number of long-presses needed from default TEXT mode
DISPLAY_MODES = text blocks vertical_blocks clock ring hourglass binary \
                radial hex matrix water_level spiral_out spiral_in \
                percent percent_remaining

# Timing settings (adjust if needed for slower systems)
INSTALL_DELAY = 5
BUTTON_DELAY = 0.5
SETTLE_DELAY = 2

# Run in emulator (default: basalt)
# Usage: make emulator [PLATFORM=basalt]
PLATFORM ?= basalt
emulator: build
	@echo "Launching $(PLATFORM) emulator..."
	pebble install --emulator $(PLATFORM)

# Shortcuts for specific platforms
emulator-aplite: build
	pebble install --emulator aplite

emulator-basalt: build
	pebble install --emulator basalt

emulator-chalk: build
	pebble install --emulator chalk

emulator-diorite: build
	pebble install --emulator diorite

emulator-emery: build
	pebble install --emulator emery

# Take a screenshot from running emulator
# Usage: make screenshot [NAME=myshot] [PLATFORM=basalt]
NAME ?= screenshot
screenshot:
	@mkdir -p $(SCREENSHOT_DIR)
	@echo "Taking screenshot: $(SCREENSHOT_DIR)/$(PLATFORM)-$(NAME).png"
	pebble screenshot --emulator $(PLATFORM) $(SCREENSHOT_DIR)/$(PLATFORM)-$(NAME).png

# Take screenshots from all platform emulators (must be running)
screenshot-all:
	@mkdir -p $(SCREENSHOT_DIR)
	@for platform in $(PLATFORMS); do \
		echo "Taking screenshot from $$platform..."; \
		pebble screenshot --emulator $$platform $(SCREENSHOT_DIR)/$$platform-$(NAME).png 2>/dev/null || \
			echo "  ($$platform emulator not running, skipping)"; \
	done
	@echo "Screenshots saved to $(SCREENSHOT_DIR)/"

# Kill all emulators
emulator-kill:
	@echo "Killing all Pebble emulators..."
	@pkill -f "qemu-pebble" 2>/dev/null || true
	@pkill -f "pypkjs" 2>/dev/null || true
	@echo "Done."

# =============================================================================
# Automated Visualization Screenshots
# =============================================================================
#
# These targets automate the full flow:
# 1. Install app in emulator
# 2. Wait for app to load
# 3. Cycle to desired display mode (long-press SELECT)
# 4. Start the timer (press SELECT)
# 5. Wait for animation to settle
# 6. Take screenshot
#
# Requires: xdotool (sudo apt install xdotool)
#
# Key mappings in QEMU emulator:
#   Up arrow    = Up button
#   Down arrow  = Down button
#   Return      = Select button
#   BackSpace   = Back button
#
# Usage:
#   make screenshot-mode MODE=blocks PLATFORM=basalt
#   make screenshot-all-modes PLATFORM=basalt
# =============================================================================

# Long press duration in seconds
LONG_PRESS_DURATION = 0.6

# Screenshot a specific mode
# Usage: make screenshot-mode MODE=blocks [PLATFORM=basalt]
MODE ?= text
screenshot-mode: build
	@mkdir -p $(SCREENSHOT_DIR)
	@echo "=== Screenshotting $(MODE) mode on $(PLATFORM) ==="
	@echo "Installing app..."
	@pebble install --emulator $(PLATFORM)
	@echo "Waiting $(INSTALL_DELAY)s for app to load..."
	@sleep $(INSTALL_DELAY)
	@# Find the emulator window
	@WID=$$(xdotool search --name qemu | tail -1); \
	if [ -z "$$WID" ]; then \
		echo "ERROR: Could not find emulator window. Is it running?"; \
		exit 1; \
	fi; \
	echo "Found emulator window: $$WID"; \
	\
	mode_idx=0; \
	for m in $(DISPLAY_MODES); do \
		if [ "$$m" = "$(MODE)" ]; then break; fi; \
		mode_idx=$$((mode_idx + 1)); \
	done; \
	echo "Cycling to mode index $$mode_idx ($(MODE))..."; \
	i=0; while [ $$i -lt $$mode_idx ]; do \
		xdotool windowactivate --sync $$WID keydown Return; \
		sleep $(LONG_PRESS_DURATION); \
		xdotool keyup Return; \
		sleep $(BUTTON_DELAY); \
		i=$$((i + 1)); \
	done; \
	\
	echo "Starting timer..."; \
	xdotool windowactivate --sync $$WID key Return; \
	\
	echo "Waiting $(SETTLE_DELAY)s for animation to settle..."; \
	sleep $(SETTLE_DELAY); \
	\
	echo "Taking screenshot..."; \
	pebble screenshot --emulator $(PLATFORM) $(SCREENSHOT_DIR)/$(PLATFORM)-$(MODE).png; \
	echo "Saved: $(SCREENSHOT_DIR)/$(PLATFORM)-$(MODE).png"

# Screenshot all display modes on current platform
# Usage: make screenshot-all-modes [PLATFORM=basalt]
screenshot-all-modes: build
	@mkdir -p $(SCREENSHOT_DIR)
	@echo "=== Screenshotting all modes on $(PLATFORM) ==="
	@for mode in $(DISPLAY_MODES); do \
		echo ""; \
		echo "--- Mode: $$mode ---"; \
		$(MAKE) --no-print-directory screenshot-mode MODE=$$mode PLATFORM=$(PLATFORM); \
	done
	@echo ""
	@echo "=== All screenshots complete! ==="
	@ls -la $(SCREENSHOT_DIR)/$(PLATFORM)-*.png

# Screenshot all modes on all platforms (the full matrix)
# Usage: make screenshot-matrix
screenshot-matrix: build
	@mkdir -p $(SCREENSHOT_DIR)
	@echo "=== Full screenshot matrix: all modes × all platforms ==="
	@for platform in $(PLATFORMS); do \
		echo ""; \
		echo "========== Platform: $$platform =========="; \
		$(MAKE) --no-print-directory screenshot-all-modes PLATFORM=$$platform; \
	done
	@echo ""
	@echo "=== Screenshot matrix complete! ==="
	@ls -la $(SCREENSHOT_DIR)/

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
	@echo "Emulator targets:"
	@echo "  make emulator            - Run in emulator (default: basalt)"
	@echo "  make emulator PLATFORM=chalk  - Run in specific platform emulator"
	@echo "  make emulator-aplite     - Run in Pebble Classic emulator (B&W)"
	@echo "  make emulator-basalt     - Run in Pebble Time emulator (color)"
	@echo "  make emulator-chalk      - Run in Pebble Time Round emulator"
	@echo "  make emulator-diorite    - Run in Pebble 2 emulator (B&W)"
	@echo "  make emulator-emery      - Run in Pebble Time 2 emulator"
	@echo "  make emulator-kill       - Kill all running emulators"
	@echo ""
	@echo "Screenshot targets:"
	@echo "  make screenshot          - Take screenshot from running emulator"
	@echo "  make screenshot-all      - Screenshot from all running emulators"
	@echo ""
	@echo "Automated visualization screenshots:"
	@echo "  make screenshot-mode MODE=blocks  - Screenshot specific visualization"
	@echo "  make screenshot-all-modes         - Screenshot all 15 visualizations"
	@echo "  make screenshot-matrix            - Screenshot all modes × all platforms"
	@echo ""
	@echo "  Available modes: text, blocks, vertical_blocks, clock, ring,"
	@echo "    hourglass, binary, radial, hex, matrix, water_level,"
	@echo "    spiral_out, spiral_in, percent, percent_remaining"
	@echo ""
	@echo "Deployment targets:"
	@echo "  make install-cloudpebble - Install to CloudPebble"
	@echo "  make install-ip IP=<ip>  - Install to device via IP address"
	@echo ""
	@echo "Development targets:"
	@echo "  make lint                - Check code for warnings"
