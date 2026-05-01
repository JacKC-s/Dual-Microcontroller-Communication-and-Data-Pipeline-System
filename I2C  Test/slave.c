#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
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
    
    // Already at TW_ST_SLA_ACK because initalization function started it
    TWDR = data;
    // initializes the data transfer
    TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
    // waits for the data transfer to be successful
    while (!(TWCR & (1 << TWINT)));

    // sending the checksum, status changes to the master asking for data
    if ((TWSR & 0xF8) == TW_ST_DATA_ACK) {
        TWDR = checksum;
        TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN);
        while (!(TWCR & (1 << TWINT)));
    }
}

uint8_t recieve_master_data() {
    // clears flag for data reception
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));

    // Checks if in data reception mode
    if ((TWSR & 0xF8) != TW_SR_DATA_ACK) {
        return 0;
    }

    return TWDR;
}

void adc_init() {
    // Uses the mux to select ADC0 (pin A0)
    ADMUX = (1 << REFS0);

    // Enabling ADC and setting prescaler to 128, the second ones are 3 bits determining the prescaler clock
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t read_adc() {
    // Sets the start signal to the adc to start measuring
    ADCSRA |= (1 << ADSC); 

    // Waiting for signal back 
    while (ADCSRA & (1 << ADSC));

    // return the 16 bit value (0-1023)
    return ADC;
}


// We will simulate a sensor by making it return random data
uint8_t readsensor() {
  uint16_t value = read_adc();
  
  // Shifting 2 bits to the right during conversion
  return (uint8_t)(value >> 2);
}

int main() {
    // Initializes slave
    i2c_init(0x18);
    adc_init();
    uint8_t command = 0;

    while (1) {
        // wait for activity
        while(!(TWCR & (1 << TWINT)));

        // determine if it is in SR or ST
        uint8_t status = (TWSR & 0xF8);

        if (status == TW_SR_SLA_ACK) {
            command = recieve_master_data();
        } else if (status == TW_ST_SLA_ACK) {
            if (command == 0x24){
                send_to_master(readsensor()); // Sends photoresistor level back
            }
        } else {
            // clear flag for reduncancy
            TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
        }

        // Resets TWCR for next condition
        TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    }
}