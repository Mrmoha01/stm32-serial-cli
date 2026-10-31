/* Entry point for the STM32C031C6 build. */
#include <stdbool.h>

#include "app.h"

int main(void)
{
    /* A real serial terminal does not echo what you type, so the CLI does it. */
    app_init(true);

    for (;;) {
        app_poll();   /* process bytes the UART interrupt put in the ring buffer */
    }
}
