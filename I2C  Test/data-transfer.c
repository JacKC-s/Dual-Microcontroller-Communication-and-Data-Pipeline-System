#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <util/twi.h>

#define F_CPU 16000000UL //Defined cpu frequency
#define SDA_PIN 2
#define SCL_PIN 3
#define MAX_RETRIES 5

void i2c_init()  {
    // Pins are handeled by TWI library
    TWSR = 0;
    TWBR = 0x48; // Sets clock to 100kHz, standard i2c communication

}

// Found an example of uart
void uart_init(uint32_t baud) {
    // divider used for the baud rate clock, gets the speed for computer and arduino
    uint16_t ubrr_value = (F_CPU / (16 * baud) - 1);
    UBRR0H = (uint8_t)(ubrr_value >> 8); // shifts 8 bits to the right, stores to high register 
    UBRR0L = (uint8_t)ubrr_value; // Stores ubrrvalue into the low register

    // Enable transmitter, UCSR0B is the control register TXEN0 is the enable register
    UCSR0B = (1 << TXEN0);
    // Set frame format: 8 data bits, 1 stop bit
    // UCSR0C is the second control register; UCSZ01 and UCZ00 char size and word length. 1 and 1 is 8bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}


void uart_putchar(char data) {
    // Wait for empty transmit buffer; UCSR0A is the status register and UDRE0 is the data register empty
    while (!(UCSR0A & (1 << UDRE0)));
    // Put data into buffer, sends the data; UDR0 is the data register
    UDR0 = data;
}

uint8_t validate_checksum(uint8_t data) {
    // Can just return NOT data because one byte not many
    return ~data;
}

void i2c_master_write(uint8_t addr, uint8_t data) {
    // Code from the official documentation for the processor
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); // Sending Start Condition
    // Waiting for the TWINT Flag (hardware)
    while (!(TWCR & (1<<TWINT)));
    // Send address and write bit
    if ((TWSR & 0xF8) != TW_START) return;
    // Sets write mode
    TWDR = (addr << 1);
    // Clears TWINT bit and turns on the enable bit
    TWCR = (1<<TWINT) | (1<<TWEN);
    // waits for TWINT to be set (hardware)
    while (!(TWCR & (1<<TWINT)));
    // To determine if a device exists at that address
    if ((TWSR & 0xF8) != TW_MT_SLA_ACK) return;
    // Loads data into TWDR register
    TWDR = data;
    // Clears TWINT register to start data transmission, moves the data
    TWCR = (1<<TWINT) | (1<<TWEN);
    // Waits for twint flag to be set 
    while (!(TWCR & (1<<TWINT)));
    // Checks value of TWSR Register, confirms data transfer
    if ((TWSR & 0xF8)!= TW_MT_DATA_ACK) return;
    // Stops communication by transmitting stop condition 
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

uint8_t i2c_master_recieve(uint8_t addr) {
    // Data and checksum to recieve
    uint8_t data, checksum;
    for (uint8_t i = 0; i < MAX_RETRIES; i++) {
        // Startup logic from i2c_master_write
        TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
        while (!(TWCR & (1<<TWINT)));
        if ((TWSR & 0xF8) != TW_START) continue; // retries if this fails.
        
        // TWDR as read mode
        TWDR = (addr << 1) | 1;
        TWCR = (1<<TWINT) | (1<<TWEN);
        while (!(TWCR & (1<<TWINT)));
        if ((TWSR & 0xF8) != TW_MR_SLA_ACK) {
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
            _delay_ms(5);
            continue;
        }

        // turns on listening from the slave
        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
        while (!(TWCR & (1<<TWINT)));
        // Changed to master reciever
        if ((TWSR & 0xF8)!= TW_MR_DATA_ACK) continue;

        data = TWDR;

        TWCR = (1<<TWINT) | (1<<TWEN);
        while (!(TWCR & (1<<TWINT)));
        // Changed to master reciever
        if ((TWSR & 0xF8)!= TW_MR_DATA_NACK) continue;

        checksum = TWDR;

        // Stops transmisson
        TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
        
        
        // Determines if checksum is correct 
        if (validate_checksum(data) == checksum) {
            return data;
        }

        _delay_ms(5);
    }
    return 0;
}

int main() {
    i2c_init();
    uart_init(9600); // Enabled for serial communication, same as arduino ide script
    uint8_t sensor_data;
    uart_putchar('I');

    while(1) {
        //uart_putchar('S');
        i2c_master_write(0x18, 0x24);
        //uart_putchar('I');
        _delay_ms(10);

        sensor_data = i2c_master_recieve(0x18);
        //uart_putchar('C');
        uart_putchar(sensor_data);
        //uart_putchar('M');
        // going to write functionality to process the data

        _delay_ms(500); // - to poll every half second
    }
}