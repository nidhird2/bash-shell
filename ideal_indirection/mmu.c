/**
 * Ideal Indirection
 * CS 241 - Spring 2020
 */
#include "mmu.h"
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


addr32 get_pd_entry_no(addr32 virtual_address){
    return virtual_address >> 22;
}

addr32 get_pt_entry(addr32 virtual_address){
    int temp = virtual_address & 0x003ff000;
    return temp >> 12;
}

addr32 calc_offset(addr32 virtual_address){
    return virtual_address & 0xfff;
}

addr32 get_base_virtual_address(addr32 virtual_address){
    return virtual_address & 0xfffff000;
}
mmu *mmu_create() {
    mmu *my_mmu = calloc(1, sizeof(mmu));
    my_mmu->tlb = tlb_create();
    return my_mmu;
}

page_table_entry* find_pte(mmu* this, addr32 virtual_address, size_t pid){
    //size_t offset = (size_t)calc_offset(virtual_address);
    addr32 base_virtual = get_base_virtual_address(virtual_address);
    // printf("virtual address: %d\n", virtual_address);
    // printf("virtual address: %X\n", virtual_address);
    // printf("based virtual address: %X\n", base_virtual);
    // printf("pde value: %X\n", get_pd_entry_no(virtual_address));
    // printf("pte value: %X\n", get_pt_entry(virtual_address));
    // printf("offset value: %X\n", calc_offset(virtual_address));
    //check if pid is same before checking tlb
    if(pid == this->curr_pid){
        page_table_entry* pg_table_entry = tlb_get_pte(&(this->tlb), base_virtual);
        if(pg_table_entry != NULL){
            //result_buffer = result;
            //printf("present: %d\n", pg_table_entry->present);
            if(pg_table_entry->present == 0){
                //printf("GOT HERE\n");
                mmu_raise_page_fault(this);
                //ask_kernel_for_frame(pg_table_entry);
                //page_table_entry* pg_table_entry = (page_table_entry)
                //@TODO:fill in flags in pg_table_entry
                pg_table_entry->base_addr = (ask_kernel_for_frame(pg_table_entry) >> NUM_OFFSET_BITS);
                pg_table_entry->user_supervisor = true;
                pg_table_entry->dirty = 0;
                pg_table_entry->accessed = 0;
                pg_table_entry->present = 1;
                pg_table_entry->read_write = 0;
                vm_segmentation* segm = find_segment(this->segmentations[pid], virtual_address);
                if(segm->permissions && (segm->permissions & WRITE) != 0){
                        pg_table_entry->read_write = 1;
                    }
                //pg_table_entry.cache_disabled = 1;
                //pg_table_entry.write_through = 1;
                //pg_table_entry.available = 3;
                //pg_table_entry.global_page = 1;
                //pg_table_entry.page_table_attribute_index = 1;
                read_page_from_disk(pg_table_entry);
            }
            return pg_table_entry;
        }
        // else{
        //     mmu_tlb_miss(this);
        // }
    }
    else{
        // mmu_tlb_miss(this);
        this->curr_pid = pid;
        tlb_flush(&(this->tlb));
    }
    //check if invalid virtual address
    if(!address_in_segmentations(this->segmentations[pid], virtual_address)){
        //printf("NOT IN SEGM\n");
        mmu_raise_segmentation_fault(this);
        //result_buffer = NULL;
        return NULL;
    }
    page_directory* pd = this->page_directories[pid];
    page_directory_entry pd_entry = pd->entries[get_pd_entry_no(virtual_address)];
    page_table* pg_table = (page_table*)get_system_pointer_from_pde(&pd_entry);
    //@TODO: if table does not exist
    if(pg_table == NULL){
        //printf("PAGE TABLE D N E\n");
        mmu_raise_page_fault(this);
        //result_buffer = NULL;
        pd_entry.base_addr = (ask_kernel_for_frame(NULL) >> NUM_OFFSET_BITS);
        pd_entry.present = true;
        pd_entry.read_write = true;
        pd_entry.user_supervisor = true;
        pg_table = (page_table*)get_system_pointer_from_pde(&pd_entry);
        //printf("new pg table val: %p\n", pg_table);
        //ask_kernel_for_frame(pg_table);
        //pg_table = (page_table*)get_system_pointer_from_address(page_table_addr);
        //pd_entry.base_addr = base_virtual >> 12;
        //pd_entry.present = 1;
        //pd_entry.accessed = 0;
    }
    //page_table_entry* result_buffer = & (pg_table->entries[get_pt_entry(virtual_address)]);
    page_table_entry* pg_table_entry = & (pg_table->entries[get_pt_entry(virtual_address)]);

    //if pg table entry is empty
    if(pg_table_entry->base_addr == 0){
        mmu_raise_page_fault(this);
        pg_table_entry->base_addr = (ask_kernel_for_frame(pg_table_entry) >> NUM_OFFSET_BITS);
        pg_table_entry->present = true;
        pg_table_entry->read_write = true;
        pg_table_entry->user_supervisor = true;
        pg_table_entry->accessed = 0;
        //return pg_table_entry;
    }
    //check if not currently mapped to physical memory
    if(pg_table_entry->present == 0){
        //printf("GOT HERE\n");
        mmu_raise_page_fault(this);
        //ask_kernel_for_frame(pg_table_entry);
        //page_table_entry* pg_table_entry = (page_table_entry)
        //@TODO:fill in flags in pg_table_entry
        pg_table_entry->base_addr = (ask_kernel_for_frame(pg_table_entry) >> NUM_OFFSET_BITS);
        pg_table_entry->user_supervisor = true;
        pg_table_entry->dirty = 0;
        pg_table_entry->accessed = 0;
        pg_table_entry->present = 1;
        pg_table_entry->read_write = 0;
        vm_segmentation* segm = find_segment(this->segmentations[pid], virtual_address);
        if(segm->permissions && (segm->permissions & WRITE) != 0){
            pg_table_entry->read_write = 1;
        }
        //pg_table_entry.cache_disabled = 1;
        //pg_table_entry.write_through = 1;
        //pg_table_entry.available = 3;
        //pg_table_entry.global_page = 1;
        //pg_table_entry.page_table_attribute_index = 1;
        read_page_from_disk(pg_table_entry);
    }
    tlb_add_pte(&(this->tlb), base_virtual, pg_table_entry);
    //printf("got here\n");
    //printf("result buffer: %p\n", result_buffer);
    mmu_tlb_miss(this);
    return pg_table_entry;
}

void mmu_read_from_virtual_address(mmu *this, addr32 virtual_address,
                                   size_t pid, void *buffer, size_t num_bytes) {
    assert(this);
    assert(pid < MAX_PROCESS_ID);
    assert(num_bytes + (virtual_address % PAGE_SIZE) <= PAGE_SIZE);
    // TODO: Implement me!
    page_table_entry* result = find_pte(this, virtual_address, pid);
    //printf("result: %p\n", result);
    if(result == NULL){
        return;
    }
    //check permissions
    //calc offset, do reading
    result->accessed = 1;
    size_t offset = calc_offset(virtual_address);
    void* ptr = get_system_pointer_from_pte(result);
    //printf("present: %d\n", result->present);
    //printf("ptr val: %p\n", ptr);
    ptr = ((char*)ptr) + offset;
    memmove((char*)buffer, (char*)ptr, num_bytes);
    return;
}

void mmu_write_to_virtual_address(mmu *this, addr32 virtual_address, size_t pid,
                                  const void *buffer, size_t num_bytes) {
    assert(this);
    assert(pid < MAX_PROCESS_ID);
    assert(num_bytes + (virtual_address % PAGE_SIZE) <= PAGE_SIZE);
    // TODO: Implement me!
    page_table_entry* result = find_pte(this, virtual_address, pid);
    //printf("result: %p\n", result);
    if(result == NULL){
        return;
    }
    //check if no write permission
    if(result->read_write != 1){
        mmu_raise_segmentation_fault(this);
        return;
    }
    //calc offset, do the writing
    result->accessed = 1;
    result->dirty = 1;
    size_t offset = calc_offset(virtual_address);
    void* ptr = get_system_pointer_from_pte(result);
    ptr = ((char*)ptr) + offset;
    memmove((char*)ptr, (char*)buffer, num_bytes);
    //check permissions
    //memmove(result, buffer, num_bytes);
    return;
}

void mmu_tlb_miss(mmu *this) {
    this->num_tlb_misses++;
}

void mmu_raise_page_fault(mmu *this) {
    this->num_page_faults++;
}

void mmu_raise_segmentation_fault(mmu *this) {
    this->num_segmentation_faults++;
}

void mmu_add_process(mmu *this, size_t pid) {
    assert(pid < MAX_PROCESS_ID);
    addr32 page_directory_address = ask_kernel_for_frame(NULL);
    this->page_directories[pid] =
        (page_directory *)get_system_pointer_from_address(
            page_directory_address);
    page_directory *pd = this->page_directories[pid];
    this->segmentations[pid] = calloc(1, sizeof(vm_segmentations));
    vm_segmentations *segmentations = this->segmentations[pid];

    // Note you can see this information in a memory map by using
    // cat /proc/self/maps
    segmentations->segments[STACK] =
        (vm_segmentation){.start = 0xBFFFE000,
                          .end = 0xC07FE000, // 8mb stack
                          .permissions = READ | WRITE,
                          .grows_down = true};

    segmentations->segments[MMAP] =
        (vm_segmentation){.start = 0xC07FE000,
                          .end = 0xC07FE000,
                          // making this writeable to simplify the next lab.
                          // todo make this not writeable by default
                          .permissions = READ | EXEC | WRITE,
                          .grows_down = true};

    segmentations->segments[HEAP] =
        (vm_segmentation){.start = 0x08072000,
                          .end = 0x08072000,
                          .permissions = READ | WRITE,
                          .grows_down = false};

    segmentations->segments[BSS] =
        (vm_segmentation){.start = 0x0805A000,
                          .end = 0x08072000,
                          .permissions = READ | WRITE,
                          .grows_down = false};

    segmentations->segments[DATA] =
        (vm_segmentation){.start = 0x08052000,
                          .end = 0x0805A000,
                          .permissions = READ | WRITE,
                          .grows_down = false};

    segmentations->segments[TEXT] =
        (vm_segmentation){.start = 0x08048000,
                          .end = 0x08052000,
                          .permissions = READ | EXEC,
                          .grows_down = false};

    // creating a few mappings so we have something to play with (made up)
    // this segment is made up for testing purposes
    segmentations->segments[TESTING] =
        (vm_segmentation){.start = PAGE_SIZE,
                          .end = 3 * PAGE_SIZE,
                          .permissions = READ | WRITE,
                          .grows_down = false};
    // first 4 mb is bookkept by the first page directory entry
    page_directory_entry *pde = &(pd->entries[0]);
    // assigning it a page table and some basic permissions
    pde->base_addr = (ask_kernel_for_frame(NULL) >> NUM_OFFSET_BITS);
    pde->present = true;
    pde->read_write = true;
    pde->user_supervisor = true;

    // setting entries 1 and 2 (since each entry points to a 4kb page)
    // of the page table to point to our 8kb of testing memory defined earlier
    for (int i = 1; i < 3; i++) {
        page_table *pt = (page_table *)get_system_pointer_from_pde(pde);
        page_table_entry *pte = &(pt->entries[i]);
        pte->base_addr = (ask_kernel_for_frame(pte) >> NUM_OFFSET_BITS);
        pte->present = true;
        pte->read_write = true;
        pte->user_supervisor = true;
    }
}

void mmu_remove_process(mmu *this, size_t pid) {
    assert(pid < MAX_PROCESS_ID);
    // example of how to BFS through page table tree for those to read code.
    page_directory *pd = this->page_directories[pid];
    if (pd) {
        for (size_t vpn1 = 0; vpn1 < NUM_ENTRIES; vpn1++) {
            page_directory_entry *pde = &(pd->entries[vpn1]);
            if (pde->present) {
                page_table *pt = (page_table *)get_system_pointer_from_pde(pde);
                for (size_t vpn2 = 0; vpn2 < NUM_ENTRIES; vpn2++) {
                    page_table_entry *pte = &(pt->entries[vpn2]);
                    if (pte->present) {
                        void *frame = (void *)get_system_pointer_from_pte(pte);
                        return_frame_to_kernel(frame);
                    }
                    remove_swap_file(pte);
                }
                return_frame_to_kernel(pt);
            }
        }
        return_frame_to_kernel(pd);
    }

    this->page_directories[pid] = NULL;
    free(this->segmentations[pid]);
    this->segmentations[pid] = NULL;

    if (this->curr_pid == pid) {
        tlb_flush(&(this->tlb));
    }
}

void mmu_delete(mmu *this) {
    for (size_t pid = 0; pid < MAX_PROCESS_ID; pid++) {
        mmu_remove_process(this, pid);
    }

    tlb_delete(this->tlb);
    free(this);
    remove_swap_files();
}
