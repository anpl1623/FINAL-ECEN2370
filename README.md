# How to get working
1. Install STM32CubeIDE for your operating system of use, (MacOS, Windows, Linux, VM, whatever), this will allow you to compile the c files using their `gcc` bash commands.
2. Connect your STM32F429i Discovery Board to your USB I/O
3. Build and run on the target board
If unresponsive, click the reset, button, if the screen goes white, re-flash the STM32 with the compiled assembly we built on the IDE.

The CPU version only implements RNG, which means the computer makes the choice at pseudorandom, without evaluating what is already on the Connect4 board.
