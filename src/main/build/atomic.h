/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// only set_BASEPRI is implemented in device library. It does always create memory barrirer
// missing versions are implemented here

// set BASEPRI and BASEPRI_MAX register, but do not create memory barrier
__attribute__( ( always_inline ) ) static inline void __set_BASEPRI_nb(uint32_t basePri)
{
   __ASM volatile ("\tMSR basepri, %0\n" : : "r" (basePri) );
}

__attribute__( ( always_inline ) ) static inline void __set_BASEPRI_MAX_nb(uint32_t basePri)
{
   __ASM volatile ("\tMSR basepri_max, %0\n" : : "r" (basePri) );
}

#if !defined(STM32F3) && !defined(STM32F4) && !defined(STM32F7)  && !defined(XMC4500_F100x1024)/* already defined in /lib/main/CMSIS/CM4/CoreSupport/core_cmFunc.h for F4 targets */
__attribute__( ( always_inline ) ) static inline void __set_BASEPRI_MAX(uint32_t basePri)
{
    __ASM volatile ("\tMSR basepri_max, %0\n" : : "r" (basePri) : "memory" );
}
#endif

// cleanup BASEPRI restore function, with global memory barrier
static inline void __basepriRestoreMem(uint8_t *val)
{
    __set_BASEPRI(*val);
}

// set BASEPRI_MAX function, with global memory barrier, returns true
static inline uint8_t __basepriSetMemRetVal(uint8_t prio)
{
    __set_BASEPRI_MAX(prio);
    return 1;
}

// cleanup BASEPRI restore function, no memory barrier
static inline void __basepriRestore(uint8_t *val)
{
    __set_BASEPRI_nb(*val);
}

// set BASEPRI_MAX function, no memory barrier, returns true
static inline uint8_t __basepriSetRetVal(uint8_t prio)
{
    __set_BASEPRI_MAX_nb(prio);
    return 1;
}

// Run block with elevated BASEPRI (using BASEPRI_MAX), restoring BASEPRI on exit. All exit paths are handled
// Full memory barrier is placed at start and exit of block
#define ATOMIC_BLOCK(prio) for ( uint8_t __basepri_save __attribute__((__cleanup__(__basepriRestoreMem))) = __get_BASEPRI(), \
                                     __ToDo = __basepriSetMemRetVal(prio); __ToDo ; __ToDo = 0 )

// Run block with elevated BASEPRI (using BASEPRI_MAX), but do not create memory barrier.
// Be careful when using this, you must use some method to prevent optimizer form breaking things
// - lto is used for Cleanflight compilation, so function call is not memory barrier
// - use ATOMIC_BARRIER or volatile to protect used variables
// - gcc 4.8.4 does write all values in registers to memory before 'asm volatile', so this optimization does not help much
// - gcc 5 and later works as intended, generating quite optimal code
#define ATOMIC_BLOCK_NB(prio) for ( uint8_t __basepri_save __attribute__((__cleanup__(__basepriRestore))) = __get_BASEPRI(), \
                                    __ToDo = __basepriSetRetVal(prio); __ToDo ; __ToDo = 0 ) \

// ATOMIC_BARRIER
// Create memory barrier
// - at the beginning of containing block (value of parameter must be reread from memory)
// - at exit of block (all exit paths) (parameter value if written into memory, but may be cached in register for subsequent use)
// On gcc 5 and higher, this protects only memory passed as parameter (any type should work)
// this macro can be used only ONCE PER LINE, but multiple uses per block are fine

#if (__GNUC__ > 6)
#warning "Please verify that ATOMIC_BARRIER works as intended"
// increment version number if BARRIER works
// TODO - use flag to disable ATOMIC_BARRIER and use full barrier instead
// you should check that local variable scope with cleanup spans entire block
#endif

#ifndef __UNIQL
# define __UNIQL_CONCAT2(x,y) x ## y
# define __UNIQL_CONCAT(x,y) __UNIQL_CONCAT2(x,y)
# define __UNIQL(x) __UNIQL_CONCAT(x,__LINE__)
#endif

// this macro uses local function for cleanup. CLang block can be substituded
#define ATOMIC_BARRIER(data)                                            \
    __extension__ void  __UNIQL(__barrierEnd)(typeof(data) **__d) {     \
        __asm__ volatile ("\t# barrier(" #data ")  end\n" : : "m" (**__d));                          \
    }                                                                   \
    typeof(data)  __attribute__((__cleanup__(__UNIQL(__barrierEnd)))) *__UNIQL(__barrier) = &data; \
    __asm__ volatile ("\t# barrier (" #data ") start\n" : "+m" (*__UNIQL(__barrier)))


// define these wrappers for atomic operations, use gcc buildins
#define ATOMIC_OR(ptr, val) __sync_fetch_and_or(ptr, val)
#define ATOMIC_AND(ptr, val) __sync_fetch_and_and(ptr, val)