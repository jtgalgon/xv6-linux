### Stride Scheduling

Basic idea:
- Each process is assigned tickets, and the stride is inversely proportional to the number of tickets assigned, specifically calculated as ```stride = max_stride / tickets```, where max_stride is the maximum stride, which is a constant number.

- Each process has a ```pass``` value, which starts from the stride and is incremented by the stride every time the process executes for a time slot

- The scheduler schedules a runnable process with the minimum pass value to run in the next time slot.


### Copy-on-Write Forking

Basic idea:
- The goal of copy-on-write (COW) fork() is to defer allocating and copying physical memory pages for the child until the copies are actually needed, if ever.

The main call for COW is found in ```cowuvm()``` in ```kernel/vm.c```