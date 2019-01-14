### ARM Emulation in C: ARMemu

For this project you are going to write a C program that can execute ARM machine code by emulating the register state of an ARM CPU and emulating the execution of ARM instructions. We will call our emulator ARMemu (armemu).

You will not need to emulate every ARM instruction, just enough to execute the assembly programs you developed in Project02. You may have to modify/simplify some of your Project02 solutions to make them easier to emulate.

In class, I have showed you how to link assembled ARM code with compiled C code. You emulator will only need to execute assembly functions that are linked with the emulator itself.

Here are the basic ideas you need to implement:
 - A representation of the register state (r0-r15, CPSR)
 - A representation of memory (stack)
 - Given a function pointer and zero or more arguments, the ability to emulate the execution of the function
     - This could be an assembly program, or in principle a compile C program.
 - The ability retrieve the return value from the emulated function
 - Dynamic analysis of the function execution:
    - Number of instructions executed
    - Instruction counts
       - Computation (data processing)
       - Memory
       - Branches
    - Registers used for reading or writing
    - Number of branches taken
    - Number of branches not taken
    - Performance measurements comparing native execution time versus emulated execution time. Use the Linux times() library function.
    - Come up with a nice way to present the analysis data

Note that the only memory you need to emulate is stack memory. You can assume all other memory for code and data resides in the address space of the process.

**Deliverables**
 - The ARMemu code
 - Demonstration that you can execute the Project02 assembly functions. 
    - Including the recursive fib function
    - Include tests from Project02, show inputs and outputs
 - Analysis reports for each of the Project02 functions.

**Instructions you need to emulate (not complete)** 
- mov (reg or immediate)
- add (reg or immediate)
- sub (reg or immediate)
- cmp (reg or immediate)
- b, beq, bnq (others as needed)
- bl, bx
- ldr, str, ldrb
- optional: push, pop
- Others depending on what you used in your Project02 code
