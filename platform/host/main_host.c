/*
 * Entry point for the PC build.
 *
 * Every character typed (or piped in) is handed to the application exactly like
 * the UART receive interrupt does on the microcontroller.
 */
#include <stdint.h>
#include <stdio.h>

#include "app.h"

int main(void)
{
    int c;

    fprintf(stderr, "[host build] press Ctrl+D to quit\n");

    /* The terminal already echoes what you type, so the CLI must not echo too. */
    app_init(false);

    while ((c = getchar()) != EOF) {
        app_on_rx_byte((uint8_t)c);   /* same call the UART interrupt makes */
        app_poll();
    }
    printf("\n");
    return 0;
}
