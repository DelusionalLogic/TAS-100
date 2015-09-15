# TAS-100
Implementing the game [TIS-100](http://store.steampowered.com/app/370360) by [Zachtronics](http://www.zachtronics.com/) on the AVR ATMega platform, found notably on [Arduino](https://www.arduino.cc/). 
This project is purely for our entertainment, and no copyright infringement is intended. 

-----------
The instructions written for the game is converted to opcode as specified in [spec.md](spec.md).
A debugging interface over serial is specified in [proto.md](proto.md)
In the src folder the source code for the AVR is found, and in the python folder the game code to opcode is found. 

-----------
The hardware setup is intended to be a master, which can be connected to a PC to debug, and ideally 12 slaves. 
Each slave would then do the workload of a node in the game and be able to communicate with 2-4 other slaves. 
How this communication will be done is yet to be determined. 
I2C using the master as master is the current thought on the matter. 
