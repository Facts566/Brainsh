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

# Build the distributable VS Code extension into dist/ at the repo root
package:
	@../tools/build-extension.sh

# Install the extension into the running VS Code
install-extension: package
	@code --install-extension $$(ls ../dist/*.vsix | tail -1) --force

.PHONY: all install clean package install-extension