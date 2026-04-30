#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <util/twi.h>

#define SDA_PIN 2
#define SCL_PIN 3
#define MAX_RETRIES 5

void i2c_init()  {
    // Pins are handeled by TWI library
    TWSR = 0;
    TWBR = 0x48; // Sets clock to 100kHz, standard i2c communication

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
    if ((TWSR & 0xF8) != START) ERROR();
    // Sets write mode
    TWDR = (addr << 1);
    // Clears TWINT bit and turns on the enable bit
    TWCR = (1<<TWINT) | (1<<TWEN);
    // waits for TWINT to be set (hardware)
    while (!(TWCR & (1<<TWINT)));
    // To determine if a device exists at that address
    if ((TWSR & 0xF8) != MT_SLA_ACK) ERROR();
    // Loads data into TWDR register
    TWDR = data;
    // Clears TWINT register to start data transmission, moves the data
    TWCR = (1<<TWINT) | (1<<TWEN);
    // Waits for twint flag to be set 
    while (!(TWCR & (1<<TWINT)));
    // Checks value of TWSR Register, confirms data transfer
    if ((TWSR & 0xF8)!= MT_DATA_ACK) ERROR();
    // Stops communication by transmitting stop condition 
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

uint8_t i2c_master_recieve(uint8_t addr) {
    // Data and checksum to recieve
    uint8_t data, checksum;
    for (uint i = 0; i < MAX_RETRIES; i++) {
        // Startup logic from i2c_master_write
        TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
        while (!(TWCR & (1<<TWINT)));
        if ((TWSR & 0xF8) != START) continue; // retries if this fails.
        
        // TWDR as read mode
        TWDR = (addr << 1) | 1;
        TWCR = (1<<TWINT) | (1<<TWEN);
        while (!(TWCR & (1<<TWINT)));
        if ((TWSR & 0xF8) != MR_SLA_ACK) {
            TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
            _delay_ms(5);
            continue;
        }

        // turns on listening from the slave
        TWCR = (1<<TWINT) | (1<<TWEN);
        while (!(TWCR & (1<<TWINT)));
        // Changed to master reciever
        if ((TWSR & 0xF8)!= MR_DATA_NACK) continue;

        data = TWDR;

        TWCR = (1<<TWINT) | (1<<TWEN);
        while (!(TWCR & (1<<TWINT)));
        // Changed to master reciever
        if ((TWSR & 0xF8)!= MR_DATA_NACK) continue;

        checksum = TWDR;

        // Stops transmisson
        TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
        
        // Determines if checksum is correct 
        if (validate_checksum(data) == checksum) {
            return data;
        }

        _delay_ms(5);
    }
    ERROR();
    return 0;
}

int main() {
    i2c_init();
    
}