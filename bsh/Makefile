PREFIX ?= $(HOME)/.local/bin

all: bsh

bsh: bsh.cpp
	g++ bsh.cpp -o bsh

# Prefer a user-writable directory so a plain `make install` needs no password
install: bsh
	@mkdir -p $(PREFIX)
	@cp bsh $(PREFIX)/bsh
	@chmod +x $(PREFIX)/bsh
	@echo "Installed to $(PREFIX)/bsh"
	@command -v bsh >/dev/null 2>&1 && echo "Running: $$(command -v bsh)" || true

clean:
	rm -f bsh/bsh

.PHONY: all install clean