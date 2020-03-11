/**
 * Ideal Indirection
 * CS 241 - Spring 2020
 */
#pragma once
#include <stdint.h>

/**
 * In this assignment, we will be simulating a 32-bit architecture machine.
 * The virtual memory space and physical memory space of this simulated machine
 * can then be fully represented with a 32-bit integer, which we've typedef-ed
 * as an addr32.
 *
 * Note that your actual VM environment may or may not be 32-bit architecture.
 * Therefore, you will need to translate the physical addresses of this
 * simulated machine into memory addresses of your actual machine.
 *
 * The full translation process goes in these steps:
 * - simulated virtual address -> simulated physical address (to be done by you)
 * - simulated physical address -> machine's virtual address (given to you in
 *   kernel.h, this is done so that you can read and write to the process'
 *   actual memory space)
 * - machine's virtual address -> machine's physical address (done by the
 *   machine)
 */
typedef uint32_t addr32;
