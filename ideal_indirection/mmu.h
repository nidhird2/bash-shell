/**
 * Ideal Indirection
 * CS 241 - Spring 2020
 */
/**
 * mmu.h - the implementation of a Memory Management Unit (MMU).
 */
#pragma once
#include "kernel.h"
#include "page_table.h"
#include "segments.h"
#include "tlb.h"

// 32768 is the largest process id on most linux systems.
#define MAX_PROCESS_ID 32768

/**
 * Struct used to represent an MMU.
 */
typedef struct {
    /**
     * An array of page directories for all the processes. Every process has one
     * page directory, and this page directory will be created when a process is
     * created in mmu_add_process(). This is not necessarily true for virtual
     * memory in general. See page_table.h for more details.
     */
    page_directory *page_directories[MAX_PROCESS_ID];
    /**
     * An array of segmentations for all the processes. The segmentations for
     * each process will be created when a process is added in
     * mmu_add_process(). See segments.h for more details.
     */
    vm_segmentations *segmentations[MAX_PROCESS_ID];
    /**
     * A TLB that needs to be updated with every memory access. See tlb.h for
     * more details.
     */
    tlb *tlb;
    /**
     * Variables that keep track of page faults, tlb misses and segfaults.
     * Use the utilities provided to update these values.
     */
    size_t num_page_faults;
    size_t num_segmentation_faults;
    size_t num_tlb_misses;
    /**
     * Current pid that is reading or writing to memory. Changes whenever
     * there is a context switch.
     */
    size_t curr_pid;
} mmu;

/**
 * Creates a default MMU with all fields initialized on the heap.
 * Call on mmu_delete() to free all memory used.
 *
 * @return Pointer to a mmu struct
 */
mmu *mmu_create();

/**
 * The following applies to both mmu_read_from_virtual address and
 * mmu_write_to_virtual_address:
 *
 * Reads 'num_bytes' from or writes 'num_bytes' to a 'virtual_address' for the
 * process with 'pid'.
 *
 * Whenever possible you should see if the page_table entry corresponding to a
 * `virtual_address` is stored in the tlb before going through multiple page
 * tables. This is because page tables are orders of magnitude slower than the
 * cache of a tlb. Make sure to keep your cache valid by checking for context
 * switches.
 *
 * If a pointer tries to access a page of address space that's currently not
 * mapped onto physical memory, then you should raise a page_fault
 * (mmu_raise_page_fault()) (Hint: there is a useful bit in the page table
 * entry) and load that page into memory (ask_kernel_for_frame() and
 * read_page_from_disk()).
 *
 * If a program tries to access an invalid or illegal memory address, then you
 * should raise a segmentation fault (mmu_raise_segmentation_fault()).
 * A memory address is invalid if the address is not in any segmentation or if
 * the user does not have the correct permissions for it (Hint: there are
 * useful bits in the page table entry).
 *
 * You should set the appropriate flags in the page directory and page table
 * entries whenever necessary (see page_table.h).
 *
 * Note: Your tlb needs to be updated whenever this method is called.
 * Note: All page directories will exist, but not all page tables will exist.
 * Note: All reads and writes will be contained within a single page.
 */

/*
 * @param this - a pointer to an mmu struct
 * @param virtual_address - The simulated virtual address to read from
 * @param pid - The pid of the process that is performing the read
 * @param buffer - The destination buffer to place the bytes you read
 * @param num_bytes - The number of bytes to read from the given address
 */
void mmu_read_from_virtual_address(mmu *this, addr32 virtual_address,
                                   size_t pid, void *buffer, size_t num_bytes);

/*
 * @param this - a pointer to an mmu struct
 * @param virtual_address - The simulated virtual address to write to
 * @param pid - The pid of the process that is performing the write
 * @param buffer - The source buffer to obtain the bytes to write to memory
 * @param num_bytes - The number of bytes to write to the given address
 */
void mmu_write_to_virtual_address(mmu *this, addr32 virtual_address, size_t pid,
                                  const void *buffer, size_t num_bytes);

/**
 * Raises a tlb miss.
 *
 * @param this - A pointer to a mmu struct
 */
void mmu_tlb_miss(mmu *this);

/**
 * Raises a segmentation fault.
 *
 * @param this - A pointer to a mmu struct
 */
void mmu_raise_segmentation_fault(mmu *this);

/**
 * Raises a page fault (both major and minor).
 *
 * @param this - A pointer to a mmu struct
 */
void mmu_raise_page_fault(mmu *this);

/**
 * Adds a process with 'pid' to the mmu by creating its virtual memory space
 * and page directory.
 *
 * @param this - A pointer to a mmu struct
 * @param pid - The pid of the process to be registered to the mmu
 */
void mmu_add_process(mmu *this, size_t pid);

/**
 * Free all the physical memory used by a certain process given by 'pid', so
 * that other processes can use that space.
 *
 * @param this - A pointer to a mmu struct
 * @param pid - The pid of the process to be removed from the mmu
 */
void mmu_remove_process(mmu *this, size_t pid);

/**
 * Frees all memory used by the mmu.
 *
 * @param this - A pointer to a mmu struct
 */
void mmu_delete(mmu *this);
