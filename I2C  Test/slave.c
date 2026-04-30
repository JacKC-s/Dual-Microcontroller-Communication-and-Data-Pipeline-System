#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <util/twi.h>

#define SDA_PIN 2
#define SCL_PIN 3

void i2c_init(uint8_t addr)  {
    // Defines the address as the input byte
    TWAR = (addr << 1);
    TWSR = 0;
    TWBR = 0x48; // Sets clock to 100kHz, standard i2c communication
    TWCR = (1<<TWEA)|(1<<TWEN);
}

void send_to_master(uint8_t data) {
    uint8_t checksum = ~data; // checksum logic
    // Starts listening for addressing by the master
    while (!(TWCR & (1<<TWINT)));
    if ((TWSR & 0xF8) != TW_ST_SLA_ACK) ERROR();

    TWDR = data;
    // initializes the data transfer
    TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
    // waits for the data transfer to be successful
    while (!(TWCR & (1 << TWINT)));

    // sending the checksum
    if ((TWSR & 0xF8) == TW_ST_SLA_ACK) {
        TWDR = checksum;
        TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
        while (!(TWCR & (1 << TWINT)));
    }
}

uint8_t recieve_master_data() {
    // initializes the data transfer
    while (!(TWCR & (1<<TWINT)));
    // Checks if in data reception mode
    if ((TWSR & 0xF8) != TW_SR_SLA_ACK) ERROR();
    
    // clears flag for data reception
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));

    return TWDR;
}

// We will simulate a sensor by making it return random data
uint8_t readsenor() {
    // Generate a random hex value between 0x00 and 0xFF
    return (uint8_t)(rand() % 256);
}

int main() {
    // Initializes slave
    i2c_init(0x18);

    while (1) {
        // wait for activity
        while(!(TWCR & (1 << TWINT)));

        uint8_t status = (TWSR & 0xF8);

        if (status == TW_SR_SLA_ACK) {
            uint8_t incoming = recieve_master_data();
        } else if (status == TW_ST_SLA_ACK) {
            send_to_master(readsenor()); // sends random hex value back
        }

        // Resets TWCR for next condition
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    }
}