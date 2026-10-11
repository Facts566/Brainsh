# Brainsh

Brainsh is an esoteric programming language based on Brainfuck. It adds a few extra commands to make working with numbers easier, while keeping the minimal spirit of the original.

## Requirements

- `g++` (C++ compiler)
- `make`

## Building the interpreter

```bash
cd Brainsh/bsh
make
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
| `:n` | Repeat `+` n times |
| `;n` | Repeat `-` n times |
| `//` | Comment |

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