/*
 * File:    clock.c
 * Description: Main file for the hard drive clock project.
 * Author:  bgeedman
 * Date:    Aug 10. 2018
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "backgrounds.h"
#include "constants.h"
#include "i2c.h"


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
void update_ds1307(void);
uint8_t bcd2bin(uint8_t);
uint8_t bin2bcd(uint8_t);


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


struct RTCDate
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekday;
    uint8_t date;
    uint8_t month;
    uint8_t year;
} gDate;


uint8_t gCycleColor[] = {OFF, RED, PURPLE, BLUE, CYAN, GREEN, YELLOW, WHITE};
uint8_t gMode = 0;
uint8_t gModeFlag = 0;
uint8_t gPlatterPos = 0;
uint8_t gRotations = 0;
uint8_t gBackground;


/*
 * Function:    bcd2bin
 * --------------------
 *  Converts the Binary-coded decimal form of a number to the binary form.
 *  EX: 0x16 BCD
 *  ------------
 *      16 - (6 * (16 / 16)) => 0x10 == 16
 */
uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }


/*
 * Function:    bin2bcd
 * --------------------
 *  Converts the binary form of a number to the Binary-coded decimal form.
 *  EX: 18 binary == 0x12
 *  ---------------------
 *      12 + (6 * (12 / 10)) => 12 + 6 * 1 => 0x18 BCD
 */
uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }


/*
 * Function:    increment_mode
 * ---------------------------
 *  Cycles through various modes of operation and blinks a white light
 *  to indicate which mode. This function is the button handler for
 *  button 1 presses and modifies the index(gMode) for selecting the
 *  correct button handlers for other modes.
 *  Modes are in order:
 *      NORMAL, BACKGROUND EDIT, HOUR EDIT, MINUTE EDIT, SECOND EDIT
 *
 *  Modifies: gModeFlag, gMode, LED color
 */
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


/*
 * Function:    change_background
 * ------------------------------
 *  Cycles through the different backgrounds defined in 'backgrounds.h'
 *  and saves the value in EEPROM memory. This function is the button
 *  handler for button 3 when in BACKGROUND EDIT mode
 *
 *  Modifies: gBackground, *EEPROM_BACKGROUND_ADDR
 */
void change_background(void)
{
    gBackground++;
    if (gBackground >= NUM_BACKGROUNDS)
        gBackground = 0;
    eeprom_write_byte((uint8_t *)EEPROM_BACKGROUND_ADDR, gBackground);
}


/*
 * Function:    increment_hour
 * ---------------------------
 *  Increments the hour hands value and saves the value to the DS1307.
 *  This function is the button handler for button 2 when in HOUR EDIT
 *  mode.
 *
 *  Modifies: gHourHand.value, gSecondHand.value
 *  Calls: update_ds1307
 */
void increment_hour(void)
{
    gHourHand.value++;
    if (gHourHand.value > 12)
        gHourHand.value = 1;

    gSecondHand.value = 0;
    update_ds1307();
}


/*
 * Function:    change_hour_color
 * ------------------------------
 *  Cycles through the hour hand colors and saves the value in
 *  EEPROM memory. This function is the button handler for button 3
 *  when in HOUR EDIT mode.
 *
 *  Modifies: gHourHand.color, *EEPROM_HOUR_ADDR
 *
 */
void change_hour_color(void)
{
    gHourHand.color++;
    if (gHourHand.color >= NUM_COLORS)
        gHourHand.color = 0;
    eeprom_write_byte((uint8_t *)EEPROM_HOUR_ADDR, gHourHand.color);
}


/*
 * Function:    increment_minute
 * -----------------------------
 *  Increments the minute hand value and saves the value to the DS1307.
 *  This function is the button handler for button 2 when in MINUTE EDIT
 *  mode.
 *
 *  Modifies: gMinuteHand.value, gSecondHand.value
 *  Calls: update_ds1307
 */
void increment_minute(void)
{
    gMinuteHand.value++;
    if (gMinuteHand.value > 59)
        gMinuteHand.value = 0;

    gSecondHand.value = 0;
    update_ds1307();
}


/*
 * Function:    change_minute_color
 * --------------------------------
 *  Cycles through the minute hand colors and saves the value in
 *  EEPROM memory. This function is the button handler for button 3
 *  when in MINUTE EDIT mode.
 *
 *  Modifies: gMinuteHand.color, *EEPROM_MINUTE_ADDR
 */
void change_minute_color(void)
{
    gMinuteHand.color++;
    if (gMinuteHand.color >= NUM_COLORS)
        gMinuteHand.color = 0;
    eeprom_write_byte((uint8_t *)EEPROM_MINUTE_ADDR, gMinuteHand.color);
}


/*
 * Function:    change_second_color
 * --------------------------------
 *  Cycles throuh the second hand colors and saves the value in
 *  EEPROM memory. This function is the button handler for button 3
 *  when in SECOND EDIT mode.
 *
 *  Modifies: gSecondHand.color, *EEPROM_SECOND_ADDR
 */
void change_second_color(void)
{
    gSecondHand.color++;
    if (gSecondHand.color >= NUM_COLORS)
        gSecondHand.color = 0;
    eeprom_write_byte((uint8_t *)EEPROM_SECOND_ADDR, gSecondHand.color);
}


/*
 * Function:    calculate_hour_position
 * ------------------------------------
 *  Calculates the position for the hour hand. This is done by
 *  taking the modulo of the current hour, and adding the fractional
 *  value of the minute hand and multiplying by sections per hour.
 *
 *  EX:  5:20
 *  ---------
 *  ((5 % 12) + (20 / 60.0)) * 15 => (5 + .33333) * 15 => 80
 *
 *  Modifies: gHourHand.pos1, gHourHand.pos2
 */
void calculate_hour_position(void)
{
    gHourHand.pos2 = ((gHourHand.value % 12) + (gMinuteHand.value / 60.0)) * 15;
    if (gHourHand.pos2 == 0)
        gHourHand.pos1 = 179;
    else
        gHourHand.pos1 = gHourHand.pos2 - 1;
}


/*
 * Function:    calculate_minute_position
 * --------------------------------------
 *  Calculates the position for the minute hand. This is done by
 *  taking the fractional value of the minute hand and multiplying
 *  by the total number of sections.
 *
 *  EX: 45'
 *  -------
 *  (45 / 60.0) * 180 => 135
 *
 *  Modifies: gMinuteHand.pos1, gMinuteHand.pos2
 */
void calculate_minute_position(void)
{
    gMinuteHand.pos2 = (gMinuteHand.value / 60.0) * 180;
    if (gMinuteHand.pos2 == 0)
        gMinuteHand.pos1 = 179;
    else
        gMinuteHand.pos1 = gMinuteHand.pos2 - 1;
}


/*
 * Function:    calculate_second_position
 * --------------------------------------
 *  Calculates the position for the second hand. This is done by
 *  taking the fractional value of the second hand and multiplying
 *  by the total number of sections.
 *
 *  EX: 30"
 *  -------
 *  (30 / 60.0) * 180 => 90
 *
 *  Modifies: gSecondHand.pos1, gSecondHand.pos2
 */
void calculate_second_position(void)
{
    gSecondHand.pos2 = (gSecondHand.value / 60.0) * 180;
    if (gSecondHand.pos2 == 0)
        gSecondHand.pos1 = 179;
    else
        gSecondHand.pos1 = gSecondHand.pos2 - 1;
}

/*
 * Function:    set_duty_cycle
 * ---------------------------
 *  Sets the PWM duty cycle for communicating with the ESC. It
 *  calculates this by taking the width, adding 50 and dividing
 *  by the length of a single PWM tick. Adding 50 is a trick for
 *  making sure the integer division rounds the correct direction.
 *
 *  Modifies: OCR2
 */
void set_duty_cycle(uint16_t width)
{
    OCR2 = ((width + 50) / PWM_TICK_WIDTH);
}


/*
 * Function:    set_color
 * ----------------------
 *  Sets the color of the LEDs.  First turn off all the bits, and
 *  then turn on the color.
 *
 *  Modifies: PORTD
 */
void set_color(uint8_t color)
{
    PORTD &= ~(WHITE);
    PORTD |= color;
}


/*
 * Function:    init_ESC
 * ---------------------
 *  Initializes the ESC by sending the correct PWM signals. First
 *  sets up TIMER 2 for fast PWM non-inverted mode and then sends
 *  a pusle of 500 microseconds to arm the device. Wait 7 seconds,
 *  and then send pulse of 1.2 milliseconds.
 *
 *      | WGM21 | WGM20 | Mode of Operation
 *  ----------------------------------------
 *   0  | 0     | 0     | Normal
 *  ---------------------------------------
 *   1  | 0     | 1     | PWM, Phase correct
 *  ---------------------------------------
 *   2  | 1     | 0     | CTC
 *  ---------------------------------------
 *   3  | 1     | 1     | Fast PWM
 *  ---------------------------------------
 *
 *
 *  COM21 | COM20 | Description
 *  ---------------------------------------
 *   0    | 0     | Normal port operation, OC2 disconnected
 *  ---------------------------------------
 *   0    | 1     | Reserved
 *  ---------------------------------------
 *   1    | 0     | non-inverting mode
 *  ---------------------------------------
 *   1    | 1     | Inverting mode
 *  ---------------------------------------
 *
 *
 *  CS22 | CS21 | CS20 | Description
 *  ---------------------------------------
 *    0  |   0  |   0  | Timer/counter stopped
 *  ---------------------------------------
 *    0  |   0  |   1  | No Prescaling
 *  ---------------------------------------
 *    0  |   1  |   0  | 8 prescaler
 *  ---------------------------------------
 *    0  |   1  |   1  | 32 prescaler
 *  ---------------------------------------
 *    1  |   0  |   0  | 64 prescaler
 *  ---------------------------------------
 *    1  |   0  |   1  | 128 prescaler
 *  ---------------------------------------
 *    1  |   1  |   0  | 256 prescaler
 *  ---------------------------------------
 *    1  |   1  |   1  | 1024 prescaler
 *  ---------------------------------------
 *
 *  Modifies: TCCR2, LED color
 */
void init_ESC(void)
{
    TCCR2 = (1 << WGM21) | (1 << WGM20) | (1 << COM21);
    TCCR2 |= (1 << CS22) | (1 << CS21) | (1 << CS20);

    set_duty_cycle(500);

    for (uint8_t i = 0; i < 7; i++)
    {
        set_color(gCycleColor[i]);
        _delay_ms(1000);
    }

    set_color(OFF);
    set_duty_cycle(1200);
}


/*
 * Function:    read_time
 * ----------------------
 *  Reads the time from the DS1307 and sets the hand values.
 *  The data read off must be converted from Binary coded
 *  data to decimal.
 *
 *  Modifies: gSecondHand.value, gMinuteHand.value, gHourHand.value
 */
void read_time(void)
{
    i2c_start(DS1307_WRITE);
    i2c_write(DS1307_SECOND_ADDR);
    i2c_stop();

    i2c_start(DS1307_READ);
    gDate.seconds = bcd2bin(i2c_read_ack());
    gDate.minutes = bcd2bin(i2c_read_ack());
    gDate.hours = bcd2bin(i2c_read_ack());
    gDate.weekday = bcd2bin(i2c_read_ack());
    gDate.date = bcd2bin(i2c_read_ack());
    gDate.month = bcd2bin(i2c_read_ack());
    gDate.year = bcd2bin(i2c_read_nack());
    i2c_stop();

    gSecondHand.value = gDate.seconds;
    gMinuteHand.value = gDate.minutes;
    gHourHand.value = gDate.hours;
}


/*
 * Function:    update_ds1307
 * --------------------------
 *  Writes the most recent values of readtime and the changed
 *  values read when setting the time to the ds1307. First it
 *  disables the oscillator.
 *
 *  Modifies: DS1307
 */
void update_ds1307(void)
{
    i2c_start(DS1307_WRITE);
    i2c_write(DS1307_SECOND_ADDR);
    i2c_write(DS1307_OSC_STOP);
    i2c_stop();

    i2c_start(DS1307_WRITE);
    i2c_write(DS1307_SECOND_ADDR);
    i2c_write(bin2bcd(gSecondHand.value));
    i2c_write(bin2bcd(gMinuteHand.value));
    i2c_write(bin2bcd(gHourHand.value));
    i2c_write(bin2bcd(gDate.weekday));
    i2c_write(bin2bcd(gDate.date));
    i2c_write(bin2bcd(gDate.month));
    i2c_write(bin2bcd(gDate.year));
    i2c_stop();
}


/*
 * Function:    ISR for TIMER0_COMP vector
 * ---------------------------------------
 *  Sets the LED colors for the current section. This interrupt
 *  should trigger everytime the platter has advanced a section.
 *  The time this takes should be relatively consistent and is
 *  adjusted when we complete 1 revolution
 *
 *  Modifies: gPlatterPos, LED color
 */
ISR(TIMER0_COMP_vect)
{
    gPlatterPos++;
    if (gPlatterPos < RESOLUTION && !gModeFlag)
    {
        set_color(pgm_read_byte(&(gBackgrounds[gBackground][gPlatterPos])));

        if (gPlatterPos == gSecondHand.pos1 || gPlatterPos == gSecondHand.pos2)
            set_color(gCycleColor[gSecondHand.color]);
        if (gPlatterPos == gMinuteHand.pos1 || gPlatterPos == gMinuteHand.pos2)
            set_color(gCycleColor[gMinuteHand.color]);
        if (gPlatterPos == gHourHand.pos1 || gPlatterPos == gHourHand.pos2)
            set_color(gCycleColor[gHourHand.color]);
    }
}


/*
 * Function:    ISR for INT0 vector
 * --------------------------------
 *  Triggers when the hall effect sensor is activated. This interrupt
 *  disables the TIMER 0 interrupt so things are not stepping on each
 *  other. It resets the TIMER 0 counter to 0 and then re-enables the
 *  interrupt. Also, it adjusts the length of OCR0 to help ensure that
 *  full resolution sections are triggered.
 *
 *  Modifies: TIMSK, TCNT0, OCR0, gPlatterPos
 */
ISR(INT0_vect)
{
    TIMSK &= ~(1 << OCIE0);
    TCNT0 = 0;
    TIMSK |= (1 << OCIE0);
    /* Help adjust for extreme cases of slowness, like starting up */
    if (OCR0 < 160 || OCR0 > 200)
        OCR0 = 179;

    /* Too many sector triggers, slow down TIMER 0 */
    if (gPlatterPos > 180)
        OCR0++;
    /* Too few sector triggers, speed up TIMER 0 */
    else if (gPlatterPos < 180)
        OCR0--;
    gPlatterPos = 0;
}



int main(void)
{
#ifdef TIME_SET

    i2c_init();
    i2c_start(DS1307_WRITE);
    i2c_write(DS1307_SECOND_ADDR);
    i2c_write(bin2bcd(SECOND));
    i2c_write(bin2bcd(MINUTE));
    i2c_write(bin2bcd(HOUR));
    i2c_write(bin2bcd(WEEKDAY));
    i2c_write(bin2bcd(DATE));
    i2c_write(bin2bcd(MONTH));
    i2c_write(bin2bcd(YEAR));
    i2c_stop();

    DDRD |= (1 << RED_LED) | (1 << BLUE_LED) | (1 << GREEN_LED);

    while(1)
    {
        set_color(GREEN);_delay_ms(500);
        set_color(OFF);_delay_ms(500);
    }

#else

    i2c_init();
    uint8_t button_const[NUM_BUTTONS] = {BUTTON1, BUTTON2, BUTTON3};
    uint8_t buttons[NUM_BUTTONS] = {0};

    /* Read the saved background and hand colors from EEPROM memory */
    gBackground       = eeprom_read_byte((const uint8_t *)EEPROM_BACKGROUND_ADDR);
    gHourHand.color   = eeprom_read_byte((const uint8_t *)EEPROM_HOUR_ADDR);
    gMinuteHand.color = eeprom_read_byte((const uint8_t *)EEPROM_MINUTE_ADDR);
    gSecondHand.color = eeprom_read_byte((const uint8_t *)EEPROM_SECOND_ADDR);

    DDRD  |= (1 << RED_LED) | (1 << BLUE_LED) |    /* Set LED pins as outputs */
             (1 << GREEN_LED) | (1 << PWM);

    init_ESC();
    PORTD |= (1 << PD2);                         /* PD2(INT0) pullup resistor */
    MCUCR  = (1 << ISC01);                               /* Falling edge INT0 */
    GICR   = (1 << INT0);                   /* Enable external interrupt INT0 */

    TCCR0  = (1 << WGM01) | (1 << CS01);     /* TIMER 0 CTC mode, 8 prescaler */
    TIMSK |= (1 << OCIE0);                        /* Enable TIMER 0 interrupt */
    OCR0   = 179;                                   /* Initial OCR for 62 RPS */
    sei();                                           /* Enable all interrupts */

    PORTA  = 0x00;
    PORTA |= (1 << BUTTON1) |               /* Button inputs internal pullups */
             (1 << BUTTON2) |
             (1 << BUTTON3);

    while (1)
    {
        read_time();
        calculate_hour_position();
        calculate_minute_position();
        calculate_second_position();

        /* check the buttons states, with some basic debounce */
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
        _delay_ms(100);
    }

#endif

    return 0;
}
