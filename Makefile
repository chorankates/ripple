# Makefile for Pebble Timer
# Supports building, CloudPebble deployment, and IP-based deployment

.PHONY: all build clean install-cloudpebble install-ip help

# Default target
all: build

build:
	pebble build

clean:
	pebble clean
	rm -rf build/

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


# Show help message
help:
	@echo "Pebble Timer Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make build               - Build the Pebble app"
	@echo "  make clean               - Clean build artifacts"
	@echo "  make install-cloudpebble - Install to CloudPebble"
	@echo "  make install-ip IP=<ip>  - Install to device via IP address"
	@echo "  make help                - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make build"
	@echo "  make install-cloudpebble"
	@echo "  make install-ip IP=192.168.1.100"

