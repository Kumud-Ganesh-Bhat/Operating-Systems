***********************************kumud_bhat_elevator_basic_v6.c*************************
*This file is a readme describing the implementation of basic version of elevator
*The code is implemented in a C programming language using pthreads, locks, pointers etc.
*The program accepts the input arguments in the following order.
* 1) number of elevators, 2) number of floors, 3)arrival time, 4)elevator speed, 5)program simulation time, 6)random seed

*The program checks whether all the arguments are greater than zero or not and also checks whether the number of floors argument is greater than 1 or not.

* Since source floor and destination floor cannot be same, the number of floors must be greater than 1, otherwise program goes in while loop infinitely.

* There is one person thread responsible for generating threads

* The number of threads playing the role of elevators is equal to number of elevators.

* So according to input  both person thread and elevator thread will be created.

* The program has four shared/global data structures
  
   1) struct gv *statistics -> stores global information related to elevator simulation.
   2) struct person -> Data structure for storing information about a person.
   3) struct personNode -> Data structure for global doubly linked list where each node contains person information.
   4) struct elevator -> Data structure for elevator information.

* It is very important to enforce exclusive secure access to these shared global data by making use of locks in proper places.

* During program implementation, i encountered various race conditions leading to errors.

* person thread routine implementation description
  ------------------------------------------------
  Many local variables are used to store value of shared global data. By doing so we can use global data through these local variables inside the function thereby preventing segmentation faults.
  the function checks whether the program simulation time is over or not through calculate_passed_time(double) function.
  if the program simulation time is not over then the person thread routine continues its task.
  It creates a new person and adds it to global doubly linked list.
  When accessing global doubly linked list lock is used so that when this thread is in critical section no other threads can enter.
  After adding a person to global doubly linked list, this function broadcasts the message to elevator thread routine which will be conditionally waiting if the list is empty.
  Followed by this the function releases lock and comes out of critical section.
  Now lock is acquired by elevator thread routine and it enters to critical section.

*elevator thread routine implementaion description
--------------------------------------------------
  Many local variables are used to store value of shared global data thereby preventing segmentation faults occuring due to accessing global variables.
  this function checks whether the program simulation time is over or not through calculate_passed_time(double) function.
  If the program simulation time is not over then this thread uses lock and checks the following condition. 
  Conditions: whether list is empty or not and person thread is generating or not.
  If list is empty and person then conditionally block itself.

  Once the elevator thread receives conditional broadcast message from person thread then this elevator thread routine acquires lock and starts its task.
  It starts removing the persons from front of the global doubly linked list.

  elevator checks whether it is in the same floor as in persons from floor or not. If not elevator starts moving from its location to person's from floor
  elevator comes down or goes up accordingly.
  once elevator arrives to persons from floor it picks the person and then drops that person to his destination floor.
  
  sleep(elev_speed) is used to achieve elevators speed from one floor to other floor.

*During implementation, time, number of people started , number of people finished has been kept tracked. 

*At the end the program prints the simulation result. 

------------------------------------------------------------------------------------------------------------------------------
Problems encountered during implementation.

*Many segmentation faults occured due to the usage of global shared data by multiple threads.
*Was very difficult to identify the segmentation faults where it is happening 
*Was very confusing about the positions of using locks.
*Then clearly went through professor slides and understood what is race condition and all. Because earlier my program for same input , sometimes used to give output, sometimes segmentation fault.
*Later i understood this is happening due to timing of uncontrollable events and due to this program fails once in 100 times.
*Earlier my program was also stopping without terminating. Later identified that when person generation thread has completed its task, elevator thread is conditionally waiting for person generation thread to produce persons.
* To overcome this i used flag variable. if flag =0 it means person generation thread task is over. 
*It was really a great experience to understand the concepts of locks in depth through this assignment.   