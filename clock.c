#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>


unsigned int extraTime = 0;
unsigned char count = 0;

ISR(TIMER0_COMP_vect)
{
    extraTime++;
    if (extraTime > 100)
    {
        extraTime = 0;
        /* count++; */
        PORTD ^= (1 << PD3);
    }
}



int main(void)
{
    DDRD = (1 << PD3) | (1 << PD4) | (1 << PD5) | (1 << PD7);

    TCCR0 = (1 << WGM01); // sets timer0 to use CTC
    OCR0 = 156;
    TIMSK = (1 << OCIE0); // set the interrupt bit
    sei(); // sets the I-bit
    TCCR0 |= (1 << CS02) | (1 << CS00); // sets the prescaler and starts the timer




    /* _delay_ms(1000); // sleep for 1 second */
    /* TCCR2 = (1 << WGM21) | (1 << WGM20); // sets the mode to fast PWM */
    /* TCCR2 |= (1 << COM21); // Sets the PWM signal to non-inverted */
    /* TIMSK |= (1 << TOIE2); */
    /* OCR2 = 64; // should be a .5 ms duty cycle to arm */
    /* TCCR2 |= (1 << CS22) | (1 << CS20); // sets the prescaler to 128 */


    /* _delay_ms(5000); // sleep for 5 seconds */

    /* OCR2 = 128; */

    while(1)
    {
    }

    return 0;
}
