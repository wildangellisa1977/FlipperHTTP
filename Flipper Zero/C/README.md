## Usage in `C` (flipper_http.h and flipper_http.c)

| **Function Name**                           | **Return Value** | **Parameters**                                                                                             | **Description**                                                                                   |
|---------------------------------------------|------------------|------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------|
| `flipper_http_alloc`                        | `FlipperHTTP*`   | None                                                                                                       | Allocates and initializes the HTTP module. Returns a `FlipperHTTP` context if successful, otherwise `NULL`. |
| `flipper_http_free`                         | `void`           | `FlipperHTTP *fhttp`                                                                                        | Deinitializes the HTTP module, stops asynchronous RX, releases the serial handle, and frees resources. |
| `flipper_http_connect_wifi`                 | `bool`           | `FlipperHTTP *fhttp`                                                                                        | Sends a command to connect to WiFi using the provided `FlipperHTTP` context. Returns `true` if successful. |
| `flipper_http_disconnect_wifi`              | `bool`           | `FlipperHTTP *fhttp`                                                                                        | Sends a command to disconnect from the current WiFi network. Returns `true` if successful.      |
| `flipper_http_ping`                         | `bool`           | `FlipperHTTP *fhttp`                                                                                        | Sends a PING request to check if the WiFi Dev Board is connected. Returns `true` if successful.  |
| `flipper_http_scan_wifi`                    | `bool`           | `FlipperHTTP *fhttp`                                                                                        | Sends a command to scan for nearby WiFi networks. Returns `true` if the request was successful.  |
| `flipper_http_save_wifi`                    | `bool`           | `FlipperHTTP *fhttp`, `const char *ssid`, `const char *password`                                           | Saves WiFi credentials for future connections. Returns `true` if successful.                     |
| `flipper_http_ip_wifi`                      | `bool`           | `FlipperHTTP *fhttp`                                                                                        | Retrieves the IP address of the connected WiFi network. Returns `true` if successful.            |
| `flipper_http_ip_address`                   | `bool`           | `FlipperHTTP *fhttp`                                                                                        | Retrieves the IP address of the WiFi Devboard. Returns `true` if successful.                     |
| `flipper_http_list_commands`                | `bool`           | `FlipperHTTP *fhttp`                                                                                        | Sends a command to list all available commands. Returns `true` if successful.                     |
| `flipper_http_led_on`                       | `bool`           | `FlipperHTTP *fhttp`                                                                                        | Enables the LED display while processing. Returns `true` if successful.                          |
| `flipper_http_led_off`                      | `bool`           | `FlipperHTTP *fhttp`                                                                                        | Disables the LED display while processing. Returns `true` if successful.                         |
| `flipper_http_parse_json`                   | `bool`           | `FlipperHTTP *fhttp`, `const char *key`, `const char *json_data`                                           | Parses JSON data for a specified key. Returns `true` if parsing was successful.                  |
| `flipper_http_parse_json_array`             | `bool`           | `FlipperHTTP *fhttp`, `const char *key`, `int index`, `const char *json_data`                             | Parses an array within JSON data for a specified key and index. Returns `true` if successful.    |
| `flipper_http_send_data`                    | `bool`           | `FlipperHTTP *fhttp`, `const char *data`                                                                    | Sends the specified data to the server with newline termination. Returns `true` if successful.    |
| `flipper_http_rx_callback`                  | `void`           | `const char *line`, `void *context`                                                                         | Callback function for handling received data asynchronously. Handles the state of the UART.       |
| `flipper_http_get_request`                  | `bool`           | `FlipperHTTP *fhttp`, `const char *url`                                                                      | Sends a GET request to the specified URL. Returns `true` if the request was successful.            |
| `flipper_http_get_request_with_headers`     | `bool`           | `FlipperHTTP *fhttp`, `const char *url`, `const char *headers`                                              | Sends a GET request with custom headers to the specified URL. Returns `true` if successful.        |
| `flipper_http_get_request_bytes`            | `bool`           | `FlipperHTTP *fhttp`, `const char *url`, `const char *headers`                                              | Sends a GET request with custom headers and retrieves raw bytes. Returns `true` if successful.     |
| `flipper_http_post_request_with_headers`    | `bool`           | `FlipperHTTP *fhttp`, `const char *url`, `const char *headers`, `const char *payload`                        | Sends a POST request with custom headers and a payload to the specified URL. Returns `true` if successful. |
| `flipper_http_post_request_bytes`           | `bool`           | `FlipperHTTP *fhttp`, `const char *url`, `const char *headers`, `const char *payload`                        | Sends a POST request with custom headers and a payload, retrieving raw bytes. Returns `true` if successful. |
| `flipper_http_put_request_with_headers`     | `bool`           | `FlipperHTTP *fhttp`, `const char *url`, `const char *headers`, `const char *payload`                        | Sends a PUT request with custom headers and a payload to the specified URL. Returns `true` if successful. |
| `flipper_http_delete_request_with_headers`  | `bool`           | `FlipperHTTP *fhttp`, `const char *url`, `const char *headers`, `const char *payload`                        | Sends a DELETE request with custom headers and a payload to the specified URL. Returns `true` if successful. |
| `flipper_http_append_to_file`               | `bool`           | `const void *data`, `size_t data_size`, `bool start_new_file`, `char *file_path`                            | Appends received data to a file. Initializes the file path before calling. Returns `true` if successful. |
| `flipper_http_load_from_file`               | `FuriString*`    | `char *file_path`                                                                                           | Loads data from the specified file. Returns a `FuriString` containing the file data.               |
| `flipper_http_load_from_file_with_limit`    | `FuriString*`    | `char *file_path`, `size_t limit`                                                                           | Loads data from the specified file with a size limit. Returns a `FuriString` containing the file data up to the limit. |
| `flipper_http_worker`                       | `int32_t`        | `void *context`                                                                                             | Worker thread to handle UART data asynchronously. Returns `0` upon completion.                     |
| `get_timeout_timer_callback`                | `void`           | `void *context`                                                                                             | Callback function invoked when a GET request times out.                                           |
| `_flipper_http_rx_callback`                 | `void`           | `FuriHalSerialHandle *handle`, `FuriHalSerialRxEvent event`, `void *context`                                | Private callback function to handle received UART data asynchronously.                            |
| `flipper_http_process_response_async`       | `bool`           | `FlipperHTTP *fhttp`, `bool (*http_request)(void)`, `bool (*parse_json)(void)`                            | Processes requests and parses JSON data asynchronously. Returns `true` if successful.             |
| `flipper_http_loading_task`                 | `void`           | `FlipperHTTP *fhttp`, `bool (*http_request)(void)`, `bool (*parse_response)(void)`, `uint32_t success_view_id`, `uint32_t failure_view_id`, `ViewDispatcher **view_dispatcher` | Performs a task while displaying a loading screen, handling success and failure views accordingly. |
| `flipper_http_websocket_start`              | `bool`           | `FlipperHTTP *fhttp`, `const char *url`, `uint16_t port`, `const char *headers`                              | Sends a request to start a WebSocket connection to the specified URL and port using the provided headers. Returns `true` if successful. |
| `flipper_http_websocket_stop`               | `bool`           | `FlipperHTTP *fhttp`                                                                                        | Sends a request to stop the WebSocket connection. Returns `true` if successful.                    |

---

**Note:**  
- In C, `fhttp->last_response` holds the received data.
- Ensure that the `FlipperHTTP *fhttp` context is properly allocated using `flipper_http_alloc()` before invoking these functions and deallocated using `flipper_http_free()` when done.
- Callback functions handle asynchronous data reception and should be implemented as per your application's requirements.

---

### Additional Information

Before using any of the HTTP functions, allocate and initialize the `FlipperHTTP` context:

```c
FlipperHTTP *fhttp = flipper_http_alloc();
if (!fhttp) {
    // Handle allocation failure
}
```

After completing all operations, free the context:

```c
flipper_http_free(fhttp);
```