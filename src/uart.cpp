#include "uart.h"

size_t UART::available()
{
#ifdef BOARD_PICO_W
    return this->serial->available();
#elif BOARD_PICO_2W
    return this->serial->available();
#elif BOARD_VGM
    return this->serial->available();
#else
    return Serial.available();
#endif
}

void UART::begin(uint32_t baudrate)
{
#ifdef BOARD_PICO_W
    this->serial = new SerialPIO(0, 1);
    this->serial->begin(baudrate);
#elif BOARD_PICO_2W
    this->serial = new SerialPIO(0, 1);
    this->serial->begin(baudrate);
#elif BOARD_VGM
    this->serial = new SerialPIO(this->tx_pin, this->rx_pin);
    this->serial->begin(baudrate);
#else
    Serial.begin(baudrate);
#endif
}

void UART::clear_buffer()
{
    while (this->available() > 0)
    {
        this->read();
    }
}

void UART::flush()
{
#ifdef BOARD_PICO_W
    this->serial->flush();
#elif BOARD_PICO_2W
    this->serial->flush();
#elif BOARD_VGM
    this->serial->flush();
#else
    Serial.flush();
#endif
}

void UART::print(String str)
{
#ifdef BOARD_PICO_W
    this->serial->print(str);
#elif BOARD_PICO_2W
    this->serial->print(str);
#elif BOARD_VGM
    this->serial->print(str);
#else
    Serial.print(str);
#endif
}

void UART::printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
#ifdef BOARD_PICO_W
    this->serial->printf(format, args);
#elif BOARD_PICO_2W
    this->serial->printf(format, args);
#elif BOARD_VGM
    this->serial->printf(format, args);
#else
    Serial.printf(format, args);
#endif
    va_end(args);
}

void UART::println(String str)
{
#ifdef BOARD_PICO_W
    this->serial->println(str);
#elif BOARD_PICO_2W
    this->serial->println(str);
#elif BOARD_VGM
    this->serial->println(str);
#else
    Serial.println(str);
#endif
}

uint8_t UART::read()
{
#ifdef BOARD_PICO_W
    return this->serial->read();
#elif BOARD_PICO_2W
    return this->serial->read();
#elif BOARD_VGM
    return this->serial->read();
#else
    Serial.read();
#endif
}

uint8_t UART::readBytes(uint8_t *buffer, size_t size)
{
#ifdef BOARD_PICO_W
    return this->serial->readBytes(buffer, size);
#elif BOARD_PICO_2W
    return this->serial->readBytes(buffer, size);
#elif BOARD_VGM
    return this->serial->readBytes(buffer, size);
#else
    return Serial.readBytes(buffer, size);
#endif
}

String UART::read_serial_line()
{
    String receivedData = "";

#ifdef BOARD_PICO_W
    receivedData = this->serial->readStringUntil('\n');
#elif BOARD_PICO_2W
    receivedData = this->serial->readStringUntil('\n');
#elif BOARD_VGM
    receivedData = this->serial->readStringUntil('\n');
#else
    while (Serial.available() > 0)
    {
        char incomingChar = Serial.read();
        if (incomingChar == '\n')
        {
            break;
        }
        receivedData += incomingChar;
        delay(1); // Minimal delay to allow buffer to fill
    }
#endif

    receivedData.trim(); // Remove any leading/trailing whitespace

    return receivedData;
}

#ifdef BOARD_VGM
void UART::set_pins(uint8_t tx_pin, uint8_t rx_pin)
{
    this->tx_pin = tx_pin;
    this->rx_pin = rx_pin;
}
#endif

void UART::set_timeout(uint32_t timeout)
{
#ifdef BOARD_PICO_W
    this->serial->setTimeout(timeout);
#elif BOARD_PICO_2W
    this->serial->setTimeout(timeout);
#elif BOARD_VGM
    this->serial->setTimeout(timeout);
#else
    Serial.setTimeout(timeout);
#endif
}

void UART::write(const uint8_t *buffer, size_t size)
{
#ifdef BOARD_PICO_W
    this->serial->write(buffer, size);
#elif BOARD_PICO_2W
    this->serial->write(buffer, size);
#elif BOARD_VGM
    this->serial->write(buffer, size);
#else
    Serial.write(buffer, size);
#endif
}