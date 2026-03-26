#include <avr/io.h>
#include <util/delay.h>

int main(void) {
  DDRB = DDRB | (1 << DDB5);
  DDRD = DDRD | (1 << DDD5);

  while (1) {
    // Sets the 5th bit of PORTB to 1
    PORTB = PORTB | (1 << PORTB5);
    PORTD = PORTD | (1 << PORTD5);

    _delay_ms(1000);

    // Clears the 5th bit of PORTB to 0
    PORTB = PORTB & ~(1 << PORTB5);
    PORTD = PORTD & ~(1 << PORTD5);

    _delay_ms(1000);
  }
}