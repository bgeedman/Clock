#ifndef CONSTANTS_H
#define CONSTANTS_H

/* macros for converting data read/write to ISC. */
#define BCDtoDEC(x)     ((x & 0x0F) + (10 * ((x >> 4) & 0x0F)))
#define DECtoBCD(x)     ((x % 10) | ((x /10) << 4))

#define NUM_MODES       5
#define NUM_COLORS      8
#define RESOLUTION      180
#define NUM_BACKGROUNDS 10

/* LED color definitions */
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


/* Button definitions */
#define NUM_BUTTONS     3
#define BUTTON1         PA4
#define BUTTON2         PA5
#define BUTTON3         PA6


/* DS1307 definitions */
#define DS1307_WRITE        0xD0
#define DS1307_READ         0xD1
#define DS1307_SECOND_ADDR  0x00
#define DS1307_MINUTE_ADDR  0x03
#define DS1307_HOUR_ADDR    0x02
#define DS1307_DAY_ADDR     0x03
#define DS1307_DATE_ADDR    0x04
#define DS1307_MONTH_ADDR   0x05
#define DS1307_YEAR_ADDR    0x06
#define DS1307_CONTROL_ADDR 0x07


/* EEPROM address definitions */
#define EEPROM_BACKGROUND_ADDR  0x00
#define EEPROM_HOUR_ADDR        0x01
#define EEPROM_MINUTE_ADDR      0x02
#define EEPROM_SECOND_ADDR      0x03

#endif
