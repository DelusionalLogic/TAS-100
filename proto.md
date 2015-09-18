TAS-100 Debug protocol (over serial)
====================================

The TAS-100 Protocol for debugging enables the user/programmer to interface with
the TAS grid from a single endpoint.

Under certain circumstances it might be necessary to interface with a single
chip to set the unique ID. In all other cases the chips should not interfere
with each other

Packet structure
----------------

The packet structure is for the debugging interface is as follows:

| Pos bit)   | Length (Bits)   | Name      | Description                   |
| ---------- | --------------- | --------- | ----------------------------- |
| 0x00       | 0x08            | recv id   | id of the intended reciever   |
| 0x08       | 0x08            | type      | type of packet                |
| 0x0F       | 0x08            | length    | length of data in bytes       |
| 0x17       | length          | data      | data of packet                |
| 0x17-0x8F  | 0x0F            | checksum  | packet checksum (crc-16)      |

The recv id is implied in the i2c protocol. It can still be set by connecting
the debugger to just one node and sending a "Set devide ID" packet on the
broadcast channel (id=0x00)

Packet data has a maximum length of 32 bytes

Packet Types
------------

| ID     | Type      | Description        | DATA                                      |
| ------ | --------- | ------------------ | ----------------------------------------- |
| 0x01   | ACK       | Ack                | [NONE]                                    |
| 0x02   | NACK      | Not ack            | [NONE]                                    |
| 0x03   | PING      | PING               | [NONE]                                    |
| 0x04   | PONG      | PONG               | [NONE]                                    |
| 0x10   | SETID     | Set device ID      | (uint8)New id                             |
| 0x11   | SETPROG   | Set program line   | (uint8)Program line, (uint32)instructions |
| 0x20   | GETID     | Get device ID      | [NONE]                                    |
| 0x21   | GETPROG   | Get program line   | (uint8Program line                        |
| 0x22   | GETREG    | Get Register       | (uint8)Register (0 for acc, 1 for bak)    |

ACK/NACK
--------

When a message is received the checksum should be checked. If the checksum is
incorrect the packet is considered damaged and a NACK should be dispatched. This
will cause the sender to repeat the message. If the checksum is correct however
an ACK should be sent. Any client dispatching an NACK should be prepared for the
other party to not resend the packet.

Setting ID
----------

Setting the ID is simple. The programmer sends a packet to the device with the
type (SETID) and the new id, this can be a maximum of 8-bit. The programmer then
has to wait for an (ACK) packet from the device.
