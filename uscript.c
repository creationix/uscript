#include "espressif/esp_common.h"
#include <esp/uart.h>
#include <string.h>

void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());
    printf("Going into echo mode...\n");

    while(1) {
        int c = getchar();
        if(c != EOF)
            putchar(c);

        /* Easter egg: check for a quack! */
        static int quack;
        if(c == "QUACK"[quack]) {
            quack++;
            if(quack == strlen("QUACK")) {
                printf("\nQUACK\nQUACK\n");
                quack = 0;
            }
        } else {
            quack = 0;
        }
    }
}
