#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <util/twi.h>

#define SDA_PIN 2
#define SCL_PIN 3

void i2c_init()  {
    /*|= is the bitwise or operation*/
    // set sda and scl as outputs
    //DDRB is the data direction, 0 is input 1 is output
    DDRB |= (1 << SDA_PIN) | (1 << SCL_PIN); // Passes in 0000 1100 into the microncontroller which turns on pin 2, 3 to output
    // sets sda and scl as high
    // Port B output register, 0 and 1 are based off of if you want it to give high or low voltage
    // This is the default before data communication
    PORTB |= (1 << SDA_PIN) | (1 << SCL_PIN);
    // sets prescaler to one which means the clock is determined by twbr register
    TWSR = 0;
    TWBR = 0x48; // Sets clock to 100kHz, standard i2c communication

}

void i2c_master_write(uint8_t addr, uint8_t data) {
    // Code from the official documentation for the processor
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN) // Sending Start Condition
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

uint8_t i2c_master_send(uint8_t addr) {
    // Data to recieve
    uint8_t data;
    // Startup logic from i2c_master_write
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    if ((TWSR & 0xF8) != START) ERROR();
    
    // TWDR as read mode
    TWDR = (addr << 1) | 1;
    TWCR = (1<<TWINT) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT)));
    if ((TWSR & 0xF8) != MR_SLA_ACK) ERROR();

    // turns on listening from the slave
    TWCR = (1<<TWINT) | (1<<TWEN);
    while (!(TWCR & (1<<TWINT)))
    // Changed to master reciever
    if ((TWSR & 0xF8)!= MR_DATA_ACK) ERROR();

    data = TWDR;

    // Stops transmisson
    TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);

    return data;
}
