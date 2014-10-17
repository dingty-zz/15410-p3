





#include "control_block.h"
#include "linked_list.h"
list thread_queue;
uint32_t next_pid;
uint32_t next_tid;





int process_init() {


// create a list of run queues based on threads
	list_init(&thread_queue);
	next_tid = 1;
	next_pid = 1;
}

int process_create(const char* filename, ) {
	simple_elf_t se_hdr;
    elf_load_helper(&se_hdr, "init");
    lprintf("%lx",se_hdr.e_entry);
    lprintf("%d",machine_phys_frames());
}


int process_exit() {


}



