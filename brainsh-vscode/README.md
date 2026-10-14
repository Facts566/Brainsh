# Brainsh

Syntax highlighting for the **Brainsh** esoteric programming language in Visual Studio Code.

Brainsh is based on Brainfuck and adds a handful of commands for working with numbers
more comfortably:

| Command | Action |
|---------|--------|
| `>` `<` | Move the pointer |
| `+` `-` | Increment / decrement the cell |
| `.` `,` | Output / input a character |
| `[` `]` | Loop while the cell is not 0 |
| `(` `)` | Run the block once if the cell holds a digit (48-57) |
| `#` | Output a number |
| `N` `S` | Convert between a character and its number |
| `d` | Split the value into tens and ones |
| `m` | Multiply the current cell by the cell on the right |
| `r` | Put a random number from 1 to 255 into the cell |
| `:n` `;n` | Repeat `+` or `-` n times |
| `@name` | Declare and call macros |
| `%file.bsh` | Include another file, making its macros available |
| `{ … }` | Comment |

## Installation

Download the `.vsix` file from the repository and install it:

```bash
code --install-extension brainsh-0.1.0.vsix
```

Or in VS Code: **Extensions** → **⋯** menu → **Install from VSIX…**

## Usage

Create a file with the `.bsh` extension and Brainsh highlighting is applied
automatically. Existing files can be switched with **Change Language Mode**
in the status bar.

## License

MIT