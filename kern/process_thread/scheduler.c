extern uint tick;
extern list *thread_queue;

void tick(unsigned int numTicks)
{
     if (numTicks % 100 == 0) 
         ++seconds;
     if (seconds % 300 == 0)
    {
     	schedule();
     }
}

void schedule() {

}