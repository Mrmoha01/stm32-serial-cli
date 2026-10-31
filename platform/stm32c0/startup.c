/*
 * Start-up code and interrupt vector table for the STM32C031C6 (Cortex-M0+).
 *
 * What happens after reset:
 *   1. The CPU loads the stack pointer from word 0 of the vector table and jumps
 *      to the address in word 1: Reset_Handler.
 *   2. Reset_Handler copies initialised variables (.data) from flash to RAM,
 *      zeroes the uninitialised ones (.bss), then calls main().
 */
#include <stdint.h>

#include "stm32c0_regs.h"

/* Symbols created by linker.ld */
extern uint32_t _estack;   /* top of RAM */
extern uint32_t _sidata;   /* where the initial values of .data sit in flash */
extern uint32_t _sdata;    /* start of .data in RAM */
extern uint32_t _edata;    /* end of .data in RAM */
extern uint32_t _sbss;     /* start of .bss */
extern uint32_t _ebss;     /* end of .bss */

extern int main(void);

typedef void (*vector_fn_t)(void);

void Reset_Handler(void);
void Default_Handler(void);

/* Every handler is a weak alias of Default_Handler. Defining a function with the
 * same name elsewhere (see hal_stm32c0.c) silently replaces it. */
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));
void USART2_IRQHandler(void)  __attribute__((weak, alias("Default_Handler")));

/* Vector table. Entries 0-15 are the Cortex-M core; peripheral interrupts start
 * at entry 16, so interrupt number N lives at index 16 + N. Unlisted entries are 0. */
__attribute__((section(".isr_vector"), used))
const vector_fn_t g_vector_table[] = {
    [0]                  = (vector_fn_t)&_estack,   /* initial stack pointer */
    [1]                  = Reset_Handler,
    [2]                  = NMI_Handler,
    [3]                  = HardFault_Handler,
    [11]                 = SVC_Handler,
    [14]                 = PendSV_Handler,
    [15]                 = SysTick_Handler,
    [16u + USART2_IRQn]  = USART2_IRQHandler,
};

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst;

    for (dst = &_sdata; dst < &_edata; ) {
        *dst++ = *src++;
    }
    for (dst = &_sbss; dst < &_ebss; ) {
        *dst++ = 0u;
    }

    (void)main();

    for (;;) {
        /* main() should never return */
    }
}

void Default_Handler(void)
{
    for (;;) {
        /* An unexpected interrupt or fault: stop here so a debugger can see it. */
    }
}
