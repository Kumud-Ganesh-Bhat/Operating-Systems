******************************** System V version of Interprocess communication ********************************************

The main objective of this program is to implement System V version of Interprocess communication using Ring Buffer Data Structure.

In this program the Ring Buffer data structure consists of integer buffer and head,tail pointers.

The Ring Buffer area is shared between driver and application process
-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

The program consists of three functions

1) struct Ring_Buffer_Area* create_global_ring_buffer_area(size_t,int*)

This function returns pointer to shared memory segment, i.e., pointer to shared ring buffer area.  

The shared memory area is created by using shmget() system call.

Then attach operation is performed using shmat() system call. The shmat() system call returns pointer to the shared memory area. This pointer can be used to read and write from/to the shared memory area.

So this function return pointer to the shared memory area.

-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

2) do_parent_process(struct Ring_Buffer_Area*, int)

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

At the end the shared memory area will be removed i.e., detached from parent process using shmdt() system call. 

The program performs fork(). When return value is 0 the control is transferred to child process. Otherwise control is transferred to parent process

The parent process performs wait() operation for the child to terminate, thereby avoiding zombie.
---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
Compilation and execution steps

gcc system_V_shm.c

./a.out

Respective outputs will be produced.