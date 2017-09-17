************************************************POSIX version of Interprocess Communication**************************************************************************

The main objective of this program is to implement POSIX version of shared memory Interprocess Communication using Ring Buffer Data Structure

The Ring Buffer Data structure consists of integer buffer and head and tail pointers

The program consists of three functions
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
1) struct Ring_Buffer_Area* create_global_ring_buffer_area(size_t,int*)

This function returns pointer to shared memory segment, i.e., pointer to shared ring buffer area.  

The shared memory object by the process is created by using shm_open() system call.

Then the size of the shared memory object is configured using ftruncate() function. 

Then the memory map of shared memory object is performed using mmap() system call which returns pointer to the shared memory object

So this function return pointer to the shared memory area.

-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

2) do_parent_process(struct Ring_Buffer_Area*)

This function generates an random integer between 0 and 19 and adds to the buffer until the buffer is not full (the condition whether buffer is full or not is checked in while loop).

So here the parent or driver process is performing the role of producer.

Parallely the parent process keeps track of sum of the integer messages produced.

As a measure of performance the program calculates the amount of time spent by the parent process to produce integer messgaes into the buffer.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

3) do_child_process(struct Ring_Buffer_Area*)

This function consumers i.e., reads the integer messages from buffer until the buffer is not empty (the condition whether buffer is empty or not is checked in while loop).

So here the child or application process is performing the role of consumer.

Parallely the child process keeps track of sum of the integer messages consumed.

As a measure of performance the amount of time spent by the child process to consume integer messages from the buffer is calculated.

--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------  

At the end the shared memory area will be removed after consumer has accessed it by using shm_unlink() system call.

The program performs fork(). When return value is 0 the control is transferred to child process. Otherwise control is transferred to parent process

The parent process performs wait() operation for the child to terminate, thereby avoiding zombie.
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Compilation and execution steps

gcc -o posixmake posix_shm.c -lrt
./posixmake
----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
However, if the BUFFER_SIZE in the program is increased beyond 1000, say for ex: 1024, am getting segmentation fault. 
The program works fine when the BUFFER_SIZE is small
I think this problem may be due to the use of ftruncate() function. 

