#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>

#define SDA_PIN 2
#define SCL_PIN 3

void i2c_init()  {
    /*|= is the bitwise or operation*/
    // set sda and scl as outputs
    //DDRB is the data direction, 0 is input 1 is output
    DDRB |= (1 << SDA_PIN) | (1 << SCL_PIN); // Passes in 1100_10 into the microncontroller which turns on pin 2, 3 to output
    // sets sda and scl as high
    // Port B output register, 0 and 1 are based off of if you want it to give high or low voltage
    PORTB |= (1 << SDA_PIN) | (1 << SCL_PIN);
    // sets prescaler to one which means the clock is determined by twbr register
    TWSR = 0;
    TWBR = 0x0C; // Sets clock to 100kHz, standard i2c communication

}