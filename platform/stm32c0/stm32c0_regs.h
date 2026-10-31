/**
 * @file stm32c0_regs.h
 * @brief The few peripheral registers this project needs, defined by hand.
 *
 * Normally you would use ST's CMSIS header (stm32c031xx.h). Writing the
 * addresses out yourself keeps the project dependency-free and shows exactly
 * what the hardware is being asked to do. The register descriptions are in the
 * STM32C0x1 reference manual (RM0490): check this file against it when changing
 * anything.
 */
#ifndef STM32C0_REGS_H
#define STM32C0_REGS_H

#include <stdint.h>

#define REG32(address) (*(volatile uint32_t *)(address))

/* --- Reset and clock control (RCC) ---------------------------------------- */
#define RCC_BASE                0x40021000u
#define RCC_CR                  REG32(RCC_BASE + 0x00u)
#define RCC_IOPENR              REG32(RCC_BASE + 0x34u)   /* GPIO clock enable  */
#define RCC_APBENR1             REG32(RCC_BASE + 0x3Cu)   /* peripheral clocks  */

#define RCC_CR_HSIDIV_POS       11u                       /* HSI48 divider field */
#define RCC_CR_HSIDIV_MASK      (7u << RCC_CR_HSIDIV_POS)
#define RCC_IOPENR_GPIOAEN      (1u << 0)
#define RCC_APBENR1_USART2EN    (1u << 17)

#define HSI48_HZ                48000000u                 /* internal oscillator */

/* --- GPIO port A ---------------------------------------------------------- */
#define GPIOA_BASE              0x50000000u
#define GPIOA_MODER             REG32(GPIOA_BASE + 0x00u) /* pin mode, 2 bits per pin */
#define GPIOA_ODR               REG32(GPIOA_BASE + 0x14u) /* output data              */
#define GPIOA_BSRR              REG32(GPIOA_BASE + 0x18u) /* atomic set / reset       */
#define GPIOA_AFRL              REG32(GPIOA_BASE + 0x20u) /* alternate function, pins 0-7 */

#define GPIO_MODE_OUTPUT        1u
#define GPIO_MODE_ALTERNATE     2u

/* --- USART2 (connected to the Nucleo's virtual COM port on PA2 / PA3) ----- */
#define USART2_BASE             0x40004400u
#define USART2_CR1              REG32(USART2_BASE + 0x00u)
#define USART2_BRR              REG32(USART2_BASE + 0x0Cu)
#define USART2_ISR              REG32(USART2_BASE + 0x1Cu)
#define USART2_ICR              REG32(USART2_BASE + 0x20u)
#define USART2_RDR              REG32(USART2_BASE + 0x24u)
#define USART2_TDR              REG32(USART2_BASE + 0x28u)

#define USART_CR1_UE            (1u << 0)   /* USART enable               */
#define USART_CR1_RE            (1u << 2)   /* receiver enable            */
#define USART_CR1_TE            (1u << 3)   /* transmitter enable         */
#define USART_CR1_RXNEIE        (1u << 5)   /* interrupt when a byte arrives */
#define USART_ISR_ORE           (1u << 3)   /* overrun error              */
#define USART_ISR_RXNE          (1u << 5)   /* a received byte is waiting */
#define USART_ISR_TXE           (1u << 7)   /* ready to accept a new byte to send */
#define USART_ICR_ORECF         (1u << 3)   /* clear the overrun flag     */

#define USART2_IRQn             28u         /* interrupt number of USART2 */

/* --- Cortex-M0+ core: SysTick timer and interrupt controller -------------- */
#define SYST_CSR                REG32(0xE000E010u)
#define SYST_RVR                REG32(0xE000E014u)
#define SYST_CVR                REG32(0xE000E018u)
#define NVIC_ISER               REG32(0xE000E100u)

#define SYST_CSR_ENABLE         (1u << 0)
#define SYST_CSR_TICKINT        (1u << 1)   /* raise an interrupt on every tick */
#define SYST_CSR_CLKSOURCE      (1u << 2)   /* run from the CPU clock           */

#endif /* STM32C0_REGS_H */
