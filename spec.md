TIS-100 Emulated Opcode specification document

Emulation things
----------------
Since the emulator will be running on the AVR ATMega platform the memory will be
stored in the little endian format. This is why opcodes are stored in the least
significant bits of the instruction.

Format
------
Instructions are 16-32 bits long, with the first 16 bits dictating if the latter 16
will be used.

 * The lower 4 bits are opcode
 * The highest bit is set the b value is kept in the next word
 * The lower 4 bits following the opcode is the a value. This is extended to the
   lower 11 bits following the opcode if the b value is kept in the next word or
   not used for this instruction
 * The 7 bits following the a value is the b value.

The different layouts have the following formats:

| e       | bytes                               |
|---------|-------------------------------------|
| set     | `eaaaaaaaaaaaoooo ?bbbbbbbbbbbbbbb` |
| not set | `ebbbbbbbaaaaoooo`                  |

The values of a and b are interpreted as specified in the table below

| Value     | Description                           |
|-----------|---------------------------------------|
| 0x01      | ACC                                   |
| 0x02      | NIL                                   |
| 0x03-0x06 | LEFT, RIGHT, UP, DOWN (In that order) |
| 0x07      | ANY                                   |
| 0x08      | LAST                                  |
| 0x09      | NIL                                   |
| 0x0A-0x3F | Literal values (-20..33) (only for b) |

When the instructions are executed the a and b values are used as source ([SRC])
and destination ([DST]). a is always [DST] while b is [SRC] or [INSTR] depending
on the opcode. [DST] can not be a literal. (Differs from the game)

| Name   | Opcode   | Params         |
| ------ | -------- | -------------- |
| MOV    | 0x1      | [SRC], [DST]   |
| ADD    | 0x2      | [SRC]          |
| SUB    | 0x3      | [SRC]          |
| NEG    | 0x4      |                |
| SWP    | 0x5      |                |
| SAV    | 0x6      |                |
| JMP    | 0x7      | [INSTR]        |
| JEZ    | 0x8      | [INSTR]        |
| JNZ    | 0x9      | [INSTR]        |
| JGZ    | 0xA      | [INSTR]        |
| JLZ    | 0xB      | [INSTR]        |
| JRO    | 0xC      | [SRC]          |

The J?? opcodes jump in the program execution. [INSTR] is an absolute value to
jump to, while JRO takes a value relative to the JRO instruction itself.

Size
----
Just like the game every node can hold a maximum of 16 instructions. The J
series of instructions will therefore never be split into 32 bits.
