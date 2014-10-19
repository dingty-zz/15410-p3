#include <syscall.h>

/*To-do: need to set the entry to readable too*/
pgt_entry* copy_pgt_entry(pgt_entry* entry)
{
	//base case
	if (entry == NULL) return NULL;
	//recursive case
	pgt_entry* new_entry = malloc(sizeof(pgt_entry));
	new_entry->virtual_addr = entry->virtual_addr;
	new_entry->phys_addr = entry->phys_addr;
	new_entry->left = copy_pgt_entry(entry->left);
	new_entry->right = copy_pgt_entry(entry->right);
}

pgt* copy_pgt(pgt* parent_pgt)
{
	pgt* child_pgt = (pgt*)malloc(sizeof(pgt));
	child_pgt->head = copy_pgt_entry(parent_pgt->head);
	return child_pgt;
}

int fork(void)
{
	//parent_pcb
	//COW
	PCB* child_pcb = (PCB*)malloc(sizeof(PCB));
	//1. copy the parent's page tables
	pgt* parent_pgt = parent_pcb -> page_table;
	pgt* child_pgt = copy_pgt(parent_pgt);
	child_pcb->page_table = child_pgt;
	//2. the assignment of a unique process 
	//descriptor struct, task_struct, for the child. 
	int curTid = gettid();

	//3. put both processes in the scheduling waiting queue
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
