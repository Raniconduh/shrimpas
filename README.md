# shrimpas

Assembler for 16 bit SHRIMP.

## Usage

Enter `shrimpas FILE` to assemble `FILE` into a binary output file named
`out.bin`.

Optionally, pass the `--ascii` flag **after** the filename to output ASCII text
into `out.bin` instead of raw binary.

## Registers

The assembler defines the following register names:

* `r0`-`r15`: Registers 0x0-0xF
* `rz`: Register 0x0 (zero register)
* `sp`: Register 0xF (stack pointer)

## Instructions

The following opcodes are accepted:

* `add`
* `sub`
* `umul` (unsigned)
* `smul` (signed)
* `udiv` (unsigned)
* `sdiv` (signed)
* `sha`
* `umod` (unsigned)
* `smod` (signed)
* `and`
* `or`
* `xor`
* `not`
* `shl`
* `rol`
* `shr`
* `ror`
* `jmp`
* `call`
* `ret`
* `reti`
* `int`
* `mov`
* `ld`
* `st`
* `ldflg`
* `stflg`

### Flags

Flags are appended to the opcode, but prefixed with a decimal. An example would
be:

```asm
call.nz label
```

## Directives

The assembler defines the following directives:

* `.ORIGIN: x`: Use `x` as the origin address (`0x0` by default)
* `.WORD: x`: Place the word `x` at the current location
* `.ZWORDS: x`: Place a block of `x` zero words at the current location


## Restrictions

Everything is case insensitive. This includes labels, directives, instructions,
etc.

Label names can be up to 15 characters long. They must start with either an
underscore or alphabetic character and may then contain any underscore or
alphanumeric character.

Labels cannot occur on the same line as anything else. This means that the
following is invalid:

```asm
label: .word: 0xFF
```

Though the following is valid:

```asm
label:
.word: 0xFF
```

Numbers are accepted in the following formats:

* Decimal: `abc...` where a is not zero
* Octal: `0abc...`
* Hexadecimal: `0xabc...` or `0Xabc...`

Behavior is undefined for numbers of invalid format.
