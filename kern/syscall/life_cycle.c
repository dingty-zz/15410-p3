#include <syscall.h>

/*To-do: need to set the entry to readable too*/
// pgt_entry* copy_pgt_entry(pgt_entry* entry)
// {
// 	//base case
// 	if (entry == NULL) return NULL;
// 	//recursive case
// 	pgt_entry* new_entry = malloc(sizeof(pgt_entry));
// 	new_entry->virtual_addr = entry->virtual_addr;
// 	new_entry->phys_addr = entry->phys_addr;
// 	new_entry->left = copy_pgt_entry(entry->left);
// 	new_entry->right = copy_pgt_entry(entry->right);
// }

// pgt* copy_pgt(pgt* parent_pgt)
// {
// 	pgt* child_pgt = (pgt*)malloc(sizeof(pgt));
// 	child_pgt->head = copy_pgt_entry(parent_pgt->head);
// 	return child_pgt;
// }

/* two more things to do: 1. copy page table 2. iret*/
int fork(void)
{
	// //parent_pcb
	// //COW
	// PCB* child_pcb = (PCB*)malloc(sizeof(PCB));
	// //1. ecopy the parent's page tables
	// pgt* parent_pgt = parent_pcb -> page_table;
	// pgt* child_pgt = copy_pgt(parent_pgt);
	// child_pcb->page_table = child_pgt;
	// //2. the assignment of a unique process 
	// //descriptor struct, task_struct, for the child. 
	// int curTid = gettid();

	// //3. put both processes in the scheduling waiting queue


	//---------------------------------------------------------------
	PCB* child_pcb = (PCB*)malloc(sizeof(PCB));
	TCB* child_tcb = (TCB*)malloc(sizeof(TCB));
	PCB* parent_pcb = current_thread -> pcb;
	//step 1: check if multi threaded; then no permission to fork;
	//to be done. We should add count of threads in pcb
	//.........

	//step 2: set up the thread control block;
	child_tcb -> pcb = child_pcb;
	child_tcb -> tid = next_tid;
	next_tid++;
	child_tcb -> state = THREAD_RUNNING;
	child_tcb -> registers = parent_tcb -> registers;
	child_tcb -> all_threads = {NULL,NULL};


	//step 3: set up the process control block;
	child_pcb -> special = 0;
	child_pcb -> ppid = parent_pcb -> ppid;
	child_pcb -> pid = next_pid;
	next_pid++;
	child_pcb -> state = PROCESS_RUNNING;
	child_pcb -> thread = child_tcb;

	//return values are different;
	child_tcb -> registers.eax = 0;
	current_thread -> registers.eax = child_pcb -> pid;

	
	//insert child to the list of threads and processes
	list_insert_last(process_queue,child_pcb);
	list_insert_last(thread_queue,child_tcb);
	list_insert_last(thread_queue,parent_tcb);

	return 0;
}

int exec(char *execname, char *argvec[])
{
	return -1;
}

void set_status(int status)
{
	return;
}

volatile int placate_the_compiler;
void vanish(void)
{
	int blackhole = 867-5309;

	blackhole ^= blackhole;
	blackhole /= blackhole;
	*(int *) blackhole = blackhole; /* won't get here */
	while (1)
		++placate_the_compiler;
}

int wait(int *status_ptr)
{
	return -1;
}


void task_vanish(int status)
{
	status ^= status;
	status /= status;
	while (1)
		continue;
}
