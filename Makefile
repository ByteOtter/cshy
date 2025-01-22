.PHONY: all setup build install clean

all: setup build install

help:
	@echo "Available targets:"
	@echo "    all - Build and install cshy."
	@echo "    setup - Setup build directory."
	@echo "    build - Build cshy."
	@echo "    install - Install cshy."
	@echo "    clean - Clean build directory"

setup:
	@if [ ! -d "build" ]; then \
		echo "Creating build dir..."; \
		mkdir build; \
	fi
	cd build && cmake ..

build:
	@echo "Building cshy..."
	cd build && cmake --build .

install:
	@echo "Installing cshy..."
	cd build && cmake --install .

clean:
	@echo "Cleaning build dir..."
	rm -rf build/*
