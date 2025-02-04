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

    // Try to wait for pong response.
    if (!flipper_http_ping(fhttp))
    {
        FURI_LOG_E(TAG, "Failed to ping the device");
        return -1;
    }
    uint8_t counter = 10;
    while (fhttp.state == INACTIVE && --counter > 0)
    {
        FURI_LOG_D(TAG, "Waiting for PONG");
        furi_delay_ms(100);
    }
    if (counter == 0)
    {
        FURI_LOG_E(TAG, "Failed to receive PONG response");
        return -1;
    }

    // Setup/send request
    if (!flipper_http_get_request_with_headers(fhttp, "https://catfact.ninja/fact", "{\"Content-Type\":\"application/json\"}"))
    {
        FURI_LOG_E(TAG, "Failed to send GET request");
        return -1;
    }
    furi_timer_start(fhttp->get_timeout_timer, TIMEOUT_DURATION_TICKS);
    fhttp->state = RECEIVING;
    while (fhttp->state == RECEIVING && furi_timer_is_running(fhttp->get_timeout_timer) > 0)
    {
        // Wait for the request to be received
        furi_delay_ms(100);
    }
    furi_timer_stop(fhttp->get_timeout_timer);

    // Print the response (you can parse the JSON here)
    FURI_LOG_I(TAG, "Received response: %s", fhttp->last_response);

    /* Using JSMN library

    char *fact = get_json_value(fhttp->last_response, "fact");
    if (fact)
    {
        FURI_LOG_I(TAG, "Cat fact: %s", fact);
    }
    else
    {
        FURI_LOG_E(TAG, "Failed to parse cat fact");
    }

    */

    /* Using Devboard JSON parser

    char last_data[256];
    snprintf(last_data, sizeof(last_data), "%s", fhttp->last_response);

    memset(fhttp->last_response, 0, strlen(fhttp->last_response));
    fhttp->last_response = NULL;

    if(!flipper_http_parse_json(fhttp, "fact", last_data))
    {
        FURI_LOG_E(TAG, "Failed to parse cat fact");
    }
    while(!fhttp->last_response)
    {
        furi_delay_ms(100);
    }

    FURI_LOG_I(TAG, "Cat fact: %s", fhttp->last_response);
    */

    // Deinitialize UART
    flipper_http_free(fhttp);

    return 0;
}