/*
 * HAL implementation for the STM32C031C6 (ST Nucleo-C031C6, also simulated by Wokwi).
 *
 *   User LED (LD4)  -> PA5
 *   Serial          -> USART2 on PA2 (TX) / PA3 (RX), 115200 baud, 8N1.
 *                      On the Nucleo board this is the ST-LINK virtual COM port.
 *   Millisecond tick-> SysTick
 *
 * The clock is left at its reset default (HSI48 divided by the HSIDIV setting).
 * The system clock is read from the hardware at start-up, so the baud rate is
 * computed correctly whatever the divider is.
 */
#include "app.h"
#include "hal.h"
#include "stm32c0_regs.h"
#include "temp_sim.h"

#define UART_BAUD      115200u
#define LED_PIN        5u
#define UART_TX_PIN    2u
#define UART_RX_PIN    3u
#define AF_USART2      1u       /* alternate function number of USART2 on PA2/PA3 */

static volatile uint32_t s_millis;

/* System clock after reset: HSI48 / 2^HSIDIV. */
static uint32_t sysclk_hz(void)
{
    uint32_t divider_exp = (RCC_CR & RCC_CR_HSIDIV_MASK) >> RCC_CR_HSIDIV_POS;

    return HSI48_HZ >> divider_exp;
}

static void gpio_set_mode(uint32_t pin, uint32_t mode)
{
    GPIOA_MODER = (GPIOA_MODER & ~(3u << (pin * 2u))) | (mode << (pin * 2u));
}

static void gpio_set_alternate(uint32_t pin, uint32_t af)   /* pins 0-7 only */
{
    GPIOA_AFRL = (GPIOA_AFRL & ~(0xFu << (pin * 4u))) | (af << (pin * 4u));
}

void hal_init(void)
{
    uint32_t clock = sysclk_hz();

    /* Clocks for GPIOA and USART2. The read-back makes sure the enable took effect. */
    RCC_IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC_APBENR1 |= RCC_APBENR1_USART2EN;
    (void)RCC_APBENR1;

    /* LED pin as output. */
    gpio_set_mode(LED_PIN, GPIO_MODE_OUTPUT);

    /* PA2 / PA3 handed over to USART2. */
    gpio_set_mode(UART_TX_PIN, GPIO_MODE_ALTERNATE);
    gpio_set_mode(UART_RX_PIN, GPIO_MODE_ALTERNATE);
    gpio_set_alternate(UART_TX_PIN, AF_USART2);
    gpio_set_alternate(UART_RX_PIN, AF_USART2);

    /* USART2: 115200 baud, 8 data bits, no parity, 1 stop bit (the reset defaults). */
    USART2_CR1 = 0u;
    USART2_BRR = (clock + UART_BAUD / 2u) / UART_BAUD;   /* rounded division */
    USART2_CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_UE;
    NVIC_ISER  = 1u << USART2_IRQn;                       /* enable its interrupt */

    /* SysTick: one interrupt per millisecond. */
    SYST_RVR = clock / 1000u - 1u;
    SYST_CVR = 0u;
    SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE;
}

void hal_uart_write(const char *data, size_t len)
{
    for (size_t i = 0u; i < len; i++) {
        while ((USART2_ISR & USART_ISR_TXE) == 0u) {
            /* wait until the transmitter can take the next byte */
        }
        USART2_TDR = (uint8_t)data[i];
    }
}

void hal_led_set(bool on)
{
    /* BSRR sets the low half to 1 and the high half to 0 in a single write. */
    GPIOA_BSRR = on ? (1u << LED_PIN) : (1u << (LED_PIN + 16u));
}

bool hal_led_get(void)
{
    return (GPIOA_ODR & (1u << LED_PIN)) != 0u;
}

uint32_t hal_millis(void)
{
    return s_millis;
}

bool hal_temp_read(int32_t *deci_celsius)
{
    /* Simulated for now. Replace this with a real reading (ADC internal sensor,
     * NTC thermistor, I2C sensor ...) and nothing else in the project changes. */
    *deci_celsius = temp_sim_deci_celsius(s_millis);
    return true;
}

const char *hal_platform_name(void)
{
    return "stm32c031c6";
}

/* ------------------------------------------------------------------------- */
/* Interrupt handlers (the names must match the vector table in startup.c)   */
/* ------------------------------------------------------------------------- */

void SysTick_Handler(void)
{
    s_millis++;
}

void USART2_IRQHandler(void)
{
    /* An overrun (a byte arrived before the previous one was read) keeps the
     * interrupt firing until it is cleared. */
    if ((USART2_ISR & USART_ISR_ORE) != 0u) {
        USART2_ICR = USART_ICR_ORECF;
    }
    /* Reading RDR returns the byte and clears RXNE. This is the ONLY thing the
     * interrupt does: hand the byte to the ring buffer and return. */
    while ((USART2_ISR & USART_ISR_RXNE) != 0u) {
        app_on_rx_byte((uint8_t)USART2_RDR);
    }
}
