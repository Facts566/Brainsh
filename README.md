# Brainsh

Brainsh is an esoteric programming language based on Brainfuck. It adds a few extra commands to make working with numbers easier, while keeping the minimal spirit of the original.

## Requirements

- `g++` (C++ compiler)
- `make`

## Building the interpreter

```bash
cd Brainsh/bsh
make

## Running a Brainsh file

bsh examples/hello.bsh

## Commands

Command Action
>       Move pointer right
<       Move pointer left
+       Increment cell
-       Decrement cell
.       Output character
,       Input character
[       Start loop
]       End loop
#       Output number
N       Subtract 48 (character → number)
S       Add 48 (number → character)
d       Split value into tens and ones
:n      Repeat + n times
;n      Repeat - n times
//      Comment

## Example

,N
>,N
<[->+<]>
#