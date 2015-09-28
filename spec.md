TIS-100 Emulated Opcode specification document

Emulation things
----------------
Since the emulator will be running on the AVR ATMega platform the memory will be
stored in the little endian format. This is why opcodes are stored in the least
significant bits of the instruction.

Format
------
Instructions are 32 bits long.

 * The lower 16 bits are the b value
 * The next 4 is the opcode
 * The highest 12 are the a value

format: `aaaaaaaaaaaaoooo bbbbbbbbbbbbbbbb`

The values of a and b are interpreted as specified in the table below

| Value       | Description                             |
| ----------- | --------------------------------------- |
| 0x01        | ACC                                     |
| 0x02        | NIL                                     |
| 0x03-0x06   | LEFT, RIGHT, UP, DOWN (In that order)   |
| 0x07        | ANY                                     |
| 0x08        | LAST                                    |
| 0x0A-0x7D8  | Literal values [-999..999] (only for b) |

When the instructions are executed the a and b values are used as source ([SRC])
and destination ([DST]). a is [DST] and the preferred parameter  while b is [SRC] depending
on the opcode. [DST] can not be a literal. (Differs from the game)

| Name   | Opcode   | Params         |
| ------ | -------- | -------------- |
| MOV    | 0x1      | [SRC], [DST]   |
| ADD    | 0x2      | [SRC]          |
| SUB    | 0x3      | [SRC]          |
| NEG    | 0x4      |                |
| NOP    | 0x5      |                |
| SWP    | 0x6      |                |
| SAV    | 0x7      |                |
| JMP    | 0x8      | [INSTR]        |
| JEZ    | 0x9      | [INSTR]        |
| JNZ    | 0xA      | [INSTR]        |
| JGZ    | 0xB      | [INSTR]        |
| JLZ    | 0xC      | [INSTR]        |
| JRO    | 0xD      | [SRC]          |

The J?? opcodes jump in the program execution. [INSTR] is an absolute value to
jump to, while JRO takes a value relative to the JRO instruction itself.
