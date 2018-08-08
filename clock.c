#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>


#define RED_LED         PD3
#define BLUE_LED        PD4
#define GREEN_LED       PD5
#define PWM_TICK_WIDTH  64
#define PWM             PD7


#define OFF             0x00
#define RED             (1 << RED_LED)
#define BLUE            (1 << BLUE_LED)
#define GREEN           (1 << GREEN_LED)
#define CYAN            (1 << BLUE_LED) | (1 << GREEN_LED)
#define PURPLE          (1 << RED_LED) | (1 << BLUE_LED)
#define YELLOW          (1 << RED_LED) | (1 << GREEN_LED)
#define WHITE           (1 << RED_LED) | (1 << BLUE_LED) | (1 << GREEN_LED)


#define RESOLUTION      180
#define TICKS   5e-7



static uint8_t colors[] = {
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
};



void init_timers(void);
void set_duty_cycle(uint16_t width);
void init_ESC(void);
void set_color(uint8_t color);


uint8_t gCount = 0;
uint8_t gPlatterPos = 0;
uint8_t gRotations = 0;


uint8_t cycle_color[] = {RED, PURPLE, BLUE, CYAN, GREEN, YELLOW, WHITE};

/*
 * Interrupt handler for dividing the clock face
 */
ISR(TIMER0_COMP_vect)
{
    gPlatterPos++;
    if (gPlatterPos < RESOLUTION)
    {
        set_color(colors[gPlatterPos]);
    }
}


/*
 * Interrupt handler that should trigger every 100 ms
 * After 10 iterations, recalculate the RPS and set OCR0 correctly
 */
ISR(TIMER1_COMPA_vect)
{
    gCount++;
    static uint8_t count = 0;
    if (gCount == 10)
    {
        TIMSK &= ~(1 << OCIE0); // turn off the section interrupt
        OCR0 = (1 / (gRotations* RESOLUTION * TICKS)); // sets the new value for the timer0
        TCNT0 = 0; // reset the timer
        gRotations = 0;
        TIMSK |= (1 << OCIE0);
        gCount = 0;

    }
    // check the button states
}


/*
 * Interrupt handler for when the sensor trips
 */
ISR(INT0_vect)
{
    gPlatterPos = 0;
    gRotations++;
}



void set_duty_cycle(uint16_t width)
{
    OCR2 = ((width + 50) / PWM_TICK_WIDTH);
}



void init_ESC(void)
{
    TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21);
    TCCR2 |= (1 << CS22) | (1 << CS21) | (1 << CS20);
    set_duty_cycle(500);
    uint8_t i;
    for (i = 0; i < 7; i++) {
        set_color(cycle_color[i]);
        _delay_ms(1000);
    }
    set_color(OFF);
    set_duty_cycle(1200);
}


void set_color(uint8_t color) {
    PORTD &= ~(WHITE);  // turn off all colors
    PORTD |= color;     // turn on the one we want
}



int main(void)
{
    DDRD |= (1 << RED_LED) | (1 << BLUE_LED) | (1 << GREEN_LED) | (1 << PWM);
    init_ESC();

    PORTD |= (1 << PD2); // should enable the pull-up
    MCUCR = (1 << ISC01); // set as dropping edge

    GICR = (1 << INT0); // turn on the external interrupt for the sensor

    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // enable 100 ms timer on timer1
    OCR1A = 20000;

    // turn on timer for the sections
    TCCR0 = (1 << WGM01) | (1 << CS01);
    TIMSK = (1 << OCIE1A);
    TIMSK |= (1 << OCIE0); // enable the section interrupt
    OCR0 = 180;

    sei();


    while(1);
    return 0;
}
