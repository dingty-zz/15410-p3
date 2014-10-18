#include "linked_list.h"
#include "control_block.h"
#include "simics.h"




unsigned int seconds;
extern list thread_queue;
TCB *current_thread;  // indicates the current runnign thread
void schedule();

void tick(unsigned int numTicks)
{
	lprintf("sdfsdf");
     if (numTicks % 100 == 0) 
         ++seconds;
     if (seconds % 300 == 0)
    {
     	schedule();
     }
}

void schedule() {

	// save current running thread


	// pop a thread from the thread_queue



	// run this thread by setting registers

}