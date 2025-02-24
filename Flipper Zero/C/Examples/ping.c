#include <flipper_http/flipper_http.h>
#define TAG "Example"
int32_t main(void *p)
{
    // Suppress unused parameter warning
    UNUSED(p);

    // Initialize the FlipperHTTP application
    FlipperHTTP *fhttp = flipper_http_alloc();
    if (!fhttp)
    {
        FURI_LOG_E(TAG, "Failed to allocate memory for FlipperHTTP");
        return -1;
    }

    if (!flipper_http_send_command(fhttp, HTTP_CMD_PING))
    {
        FURI_LOG_E(TAG, "Failed to ping the device");
        return -1;
    }

    // Try to wait for pong response.
    uint8_t counter = 10;
    while (fhttp->state == INACTIVE && --counter > 0)
    {
        FURI_LOG_D(TAG, "Waiting for PONG");
        furi_delay_ms(100);
    }

    if (counter == 0)
    {
        FURI_LOG_E(TAG, "Failed to receive PONG response");
        return -1;
    }

    // Deinitialize UART
    flipper_http_free(fhttp);

    return 0;
}