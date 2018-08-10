#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "i2c.h"

#define RED_LED         PD3
#define BLUE_LED        PD4
#define GREEN_LED       PD5
#define PWM_TICK_WIDTH  64
#define PWM             PD7


#define NUM_BUTTONS     3
#define BUTTON1         PD4
#define BUTTON2         PD5
#define BUTTON3         PD6


#define OFF             0x00
#define RED             (1 << RED_LED)
#define BLUE            (1 << BLUE_LED)
#define GREEN           (1 << GREEN_LED)
#define CYAN            (1 << BLUE_LED) | (1 << GREEN_LED)
#define PURPLE          (1 << RED_LED) | (1 << BLUE_LED)
#define YELLOW          (1 << RED_LED) | (1 << GREEN_LED)
#define WHITE           (1 << RED_LED) | (1 << BLUE_LED) | (1 << GREEN_LED)

#define RESOLUTION      180
#define TICKS           5e-7

#define NUM_BACKGROUNDS 4
#define NUM_MODES       5
#define NUM_COLORS      8


#define DS1307_WRITE    0xD0
#define DS1307_READ     0xD1

#define BCDtoDEC(x)     ((x & 0x0F) + (10 * ((x >> 4) & 0x0F)))
#define DECtoBCD(x)     ((x % 10) | ((x /10) << 4))


uint8_t gBackgrounds[NUM_BACKGROUNDS][RESOLUTION] = {
{
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF,
    OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF, OFF
},
{
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED
},
{
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN,
    GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN, GREEN
},
/* { */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, */
/*     BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE */
/* }, */
/* { */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN */
/* }, */
/* { */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, */
/*     PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE, PURPLE */
/* }, */
/* { */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, */
/*     YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW, YELLOW */
/* }, */
/* { */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE */
/* }, */
/* /1* CANDY CANE *1/ */
/* { */
/*     RED, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     RED, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     RED, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     RED, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     RED, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     RED, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     RED, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     RED, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     RED, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, CYAN, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/*     WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, */
/* }, */

{
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
    WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
    WHITE, WHITE, WHITE, WHITE, WHITE, RED, RED, RED, RED, RED,
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
    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
    BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE
},
};


/*                  FUNCTION DECLARATION                */
void set_duty_cycle(uint16_t width);
void init_ESC(void);
void set_color(uint8_t color);
void increment_mode(void);
void increment_hour(void);
void increment_minute(void);
void change_background(void);
void change_hour_color(void);
void change_minute_color(void);
void change_second_color(void);
void calculate_hour_position(void);
void calculate_minute_position(void);
void calculate_second_position(void);
void read_time(void);


/*              GLOBAL VARIABLE DECLARATION              */
void (*gButtonHandlers[NUM_MODES][NUM_BUTTONS])(void) =
{   {increment_mode, NULL, NULL},
    {increment_mode, NULL, change_background},
    {increment_mode, increment_hour, change_hour_color},
    {increment_mode, increment_minute, change_minute_color},
    {increment_mode, NULL, change_second_color}
};

typedef struct Hand
{
    uint8_t value;
    uint8_t color;
    uint8_t pos1;
    uint8_t pos2;
} Hand;

Hand gHourHand;
Hand gMinuteHand;
Hand gSecondHand;

uint8_t gCycleColor[] = {OFF, RED, PURPLE, BLUE, CYAN, GREEN, YELLOW, WHITE};
uint8_t gMode = 0;
uint8_t gModeFlag = 0;
uint8_t gPlatterPos = 0;
uint8_t gRotations = 0;
uint8_t gBackground;




void increment_mode(void)
{
    gModeFlag = 1;
    gMode++;
    if (gMode >= NUM_MODES)
    {
        set_color(WHITE);_delay_ms(100);
        set_color(OFF);_delay_ms(100);
        gMode = 0;
    }
    set_color(WHITE);_delay_ms(250);
    set_color(OFF);_delay_ms(250);
    gModeFlag = 0;
}


void change_background(void)
{
    gBackground++;
    if (gBackground >= NUM_BACKGROUNDS)
        gBackground = 0;
}

void change_hour_color(void)
{
    gHourHand.color++;
    if (gHourHand.color >= NUM_COLORS)
        gHourHand.color = 0;
}

void change_minute_color(void)
{
    gMinuteHand.color++;
    if (gMinuteHand.color >= NUM_COLORS)
        gMinuteHand.color = 0;
}

void change_second_color(void)
{
    gSecondHand.color++;
    if (gSecondHand.color >= NUM_COLORS)
        gSecondHand.color = 0;
}


void calculate_hour_position(void)
{
    gHourHand.pos2 = ((gHourHand.value % 12) + (gMinuteHand.value / 60.0)) * 15;
    if (gHourHand.pos2 == 0)
        gHourHand.pos1 = 179;
    else
        gHourHand.pos1 = gHourHand.pos2 - 1;
}

void calculate_minute_position(void)
{
    gMinuteHand.pos2 = (gMinuteHand.value / 60.0) * 180;
    if (gMinuteHand.pos2 == 0)
        gMinuteHand.pos1 = 179;
    else
        gMinuteHand.pos1 = gMinuteHand.pos2 - 1;
}

void calculate_second_position(void)
{
    gSecondHand.pos2 = (gSecondHand.value / 60.0) * 180;
    if (gSecondHand.pos2 == 0)
        gSecondHand.pos1 = 179;
    else
        gSecondHand.pos1 = gSecondHand.pos2 - 1;
}


void increment_hour(void)
{
    gHourHand.value++;
    if (gHourHand.value > 12)
        gHourHand.value = 1;

    // save the new hour to the RTC
    i2c_start(DS1307_WRITE);
    i2c_write(0x02);
    i2c_write(DECtoBCD(gHourHand.value));
    i2c_stop();
}


void increment_minute(void)
{
    gMinuteHand.value++;
    if (gMinuteHand.value > 59)
        gMinuteHand.value = 0;

    // save the new minute to the RTC
    i2c_start(DS1307_WRITE);
    i2c_write(0x00);
    i2c_write(DECtoBCD(0)); // zero out the seconds when we set the minute
    i2c_write(DECtoBCD(gMinuteHand.value));
    i2c_stop();
}


/*
 * Interrupt handler for dividing the clock face
 */
ISR(TIMER0_COMP_vect)
{
    gPlatterPos++;
    if (gPlatterPos < RESOLUTION && !gModeFlag)
    {
        set_color(gBackgrounds[gBackground][gPlatterPos]);

        if (gPlatterPos == gSecondHand.pos1 || gPlatterPos == gSecondHand.pos2)
            set_color(gCycleColor[gSecondHand.color]);
        if (gPlatterPos == gMinuteHand.pos1 || gPlatterPos == gMinuteHand.pos2)
            set_color(gCycleColor[gMinuteHand.color]);
        if (gPlatterPos == gHourHand.pos1 || gPlatterPos == gHourHand.pos2)
            set_color(gCycleColor[gHourHand.color]);
    }
}




/*
 * Interrupt handler for when the sensor trips
 */
ISR(INT0_vect)
{
    TIMSK &= ~(1 << OCIE0);
    TCNT0 = 0;
    TIMSK |= (1 << OCIE0);
    if (OCR0 < 160 || OCR0 > 200)
        OCR0 = 179;

    if (gPlatterPos > 180)
        OCR0++;
    else if (gPlatterPos < 180)
        OCR0--;

    gPlatterPos = 0;
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
    for (i = 0; i < 7; i++)
    {
        set_color(gCycleColor[i]);
        _delay_ms(1000);
    }
    set_color(OFF);
    set_duty_cycle(1200);
}


void set_color(uint8_t color)
{
    PORTD &= ~(WHITE);
    PORTD |= color;
}


void read_time(void)
{
    i2c_init();
    i2c_start(DS1307_WRITE);
    i2c_write(0x00); // set address to 0x00
    i2c_stop();

    i2c_start(DS1307_READ);
    gSecondHand.value = BCDtoDEC(i2c_read_ack());
    gMinuteHand.value = BCDtoDEC(i2c_read_ack());
    gHourHand.value = BCDtoDEC(i2c_read_nack());
    i2c_stop();
}


int main(void)
{

     uint8_t button_const[NUM_BUTTONS] = {BUTTON1, BUTTON2, BUTTON3};
     uint8_t buttons[NUM_BUTTONS] = {0};

    gBackground = 0;            // read this from EEPROM
    gHourHand.color = 1;        // read this from EEPROM
    gMinuteHand.color = 4;      // read this from EEPROM
    gSecondHand.color = 5;      // read this from EEPROM


    DDRD |= (1 << RED_LED) | (1 << BLUE_LED) | (1 << GREEN_LED) | (1 << PWM);
    init_ESC();
    PORTD |= (1 << PD2); // should enable the pull-up
    MCUCR = (1 << ISC01); // set as dropping edge
    GICR = (1 << INT0); // turn on the external interrupt for the sensor
    // section timer
    TCCR0 = (1 << WGM01) | (1 << CS01);
    TIMSK |= (1 << OCIE0);
    OCR0 = 179;
    sei();


    PORTA |= (1 << BUTTON1) | (1 << BUTTON2) | (1 << BUTTON3);


    while (1)
    {
        read_time();
        calculate_hour_position();
        calculate_minute_position();
        calculate_second_position();
        for (int i = 0; i < NUM_BUTTONS; i++)
        {
            if (PINA & (1 << button_const[i]))
            {
                if (!buttons[i])
                {
                    buttons[i] = 1;
                }
            }
            else
            {
                if (buttons[i])
                {
                    if (gButtonHandlers[gMode][i] != NULL)
                    {
                        gButtonHandlers[gMode][i]();
                    }
                    buttons[i] = 0;
                }
            }
        }
        _delay_ms(100); // after 10 loops, should i just increment the time
    }

    return 0;
}




