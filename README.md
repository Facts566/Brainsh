# Brainsh

![Иконка](bsh/brainsh.png)

Brainsh is an esoteric programming language based on Brainfuck. It adds a few extra commands to make working with numbers easier, while keeping the minimal spirit of the original.

## Editor support

Syntax highlighting for `.bsh` files is available as a VS Code extension.

Download [`dist/brainsh-0.1.0.vsix`](dist/brainsh-0.1.0.vsix) and install it:

```bash
code --install-extension dist/brainsh-0.1.0.vsix
```

Or in VS Code: **Extensions** → **⋯** → **Install from VSIX…**

To rebuild the package yourself you need [`@vscode/vsce`](https://www.npmjs.com/package/@vscode/vsce):

```bash
cd Brainsh/bsh
make package
```

## Requirements

- `g++` (C++ compiler)
- `make`

## Building the interpreter

```bash
cd Brainsh/bsh
make install
```

## Running a Brainsh file

```bash
bsh examples/hello.bsh
```

## Commands

| Command | Action |
|---------|--------|
| `>` | Move pointer right |
| `<` | Move pointer left |
| `+` | Increment cell |
| `-` | Decrement cell |
| `.` | Output character |
| `,` | Input character |
| `[` | Start loop if not 0 |
| `]` | End loop if 0 |
| `(` | Run block once if the cell holds a digit (48-57, `'0'`-`'9'`) |
| `)` | End the condition block |
| `#` | Output number |
| `N` | Subtract 48 (character → number) |
| `S` | Add 48 (number → character) |
| `d` | Split value into tens and ones |
| `m` | Multiply the current cell by the cell on the right |
| `r` | Put a random number from 1 to 255 into the cell |
| `:n` | Repeat `+` n times |
| `;n` | Repeat `-` n times |
| `@` | Declares a macro |
| `%file` | Include another file, making its macros available |
| `//` | Comment |

## Arithmetic

`m` reads the cell on the right, multiplies the two values and stores the
product back in the current cell. Like every other cell value it wraps around
at 256, so `255 * 3 = 765` becomes `253`.

```bash
:5 >:7 <m#     # 5 * 7 = 35
```

`r` puts a random number from 1 to 255 into the current cell. The generator is
seeded from the clock, so the value differs on every run.

```bash
r#             # prints some number between 1 and 255
```

## Including files

`%file.bsh` inlines another file, so the macros defined in it can be called
from the current program. The name is read until whitespace; use quotes if the
name contains spaces: `%"my lib.bsh"`. Paths are resolved relative to the
including file first, then to the working directory. Including the same file
twice is safe — it is only processed once.

```
// lib.bsh
@put = .[-]

// main.bsh
%lib.bsh
:72@put        // prints "H"
```

Macros may call other macros, including ones from included files. A macro that
calls itself is reported as an error instead of looping forever.

## Conditions

Round brackets run their block **once** if the current cell holds a digit
(48-57, the characters `'0'`-`'9'`). Otherwise the block is skipped. Unlike
`[ ]`, they do not loop.

```bash
:52(.)      // 52 = '4' is a digit, so '4' is printed
:81(.)      // 52 + 81 = 133 is not a digit, so nothing is printed
```

Combined with a loop, this lets you walk over a range of characters:

```bash
:48>:5      // start at '0', counter = 5
[
  <(. :2)>;1   // print the digit, add 2, decrement the counter
]
```

This prints `02468`.

## Example

```bash
,N
>,N
<[->+<]>
#
```
