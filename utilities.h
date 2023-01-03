/*\
|*| utilities.h
|*| 
|*| (c) 2022 Trent M. Wyatt.
|*| companion file for Reverse Geocache Box project
|*| 
\*/

#if !defined(UTILITIES_H)
#define  UTILITIES_H

#pragma   pack(0)

#define   ARRAYSZ(A)    (sizeof(A)/sizeof(*(A)))

#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
#include <TinyGPSPlus.h>
#include <PWMServo.h>
#include <CPUVolt.h>
#include <CPUTemp.h>
#include <time.h>
#include <Wire.h>

#include "eeprom_data.h"
#include "utilities.h"
#include "datetime.h"
#include "menu.h"

// ======================================================================
// Manifest Constants to Control Compile-Time Options

// SERIAL_OUTPUT
// This enables the output of serial debug output.
// If SERIAL_OUTPUT is enabled all LCD output will go
// to the serial output instead of the lcd display.
// Note: Using the Serial library adds considerable flash and SRAM usage.
// Disable if not needed for production.

// DEVELOPER_MODE
// This disables writing to the EEPROM on every attempt

// SELF_TEST
// This runs a set of routines to exercise all of the various
// subsystems in the project for testing and debugging.

// ======================================================================

// uncomment to enable serial debug output
//#define SERIAL_OUTPUT

// comment out to enable production mode
//#define DEVELOPER_MODE

// uncomment to enable SELF_TEST mode
//#define SELF_TEST

// external global variables
extern    SoftwareSerial   ss;
extern    TinyGPSPlus      gps;
extern    eeprom_data_t    config_data;

// ======================================================================
// Magic Numbers - cryptic numbers we don't want in the code

enum MagicNumber_t {
    // Pin Assignments
    // Adjust per project as your build requires.
    // 
    // !! Also see Playtune_poll.h for the 8 output pins used for sound !!
    // Current sound output pins:
    // 5, 6, 7, 8, A0, 10, 11, 12
    // Each sound output pin has a 500 ohm resistor to a common output
    // which is fed into an amplifier and then to a speaker

    DbgLedPin  = LED_BUILTIN,   // debugging LED
    SecretPin1 = A1,            // hidden input pin attached to one hinge on the box lid
    SecretPin2 = A2,            // hidden output pin attached to the other hinge on the box lid
    SrvPin     = SERVO_PIN_A,   // pin 9 from PWMServo.h
    RXPin      = 4,             // receive from GPS module
    TXPin      = 3,             // transmit to GPS module
    PwrPin     = 2,             // raise HIGH to power off

    DefaultTries = 50,          // Default number of remaining tries

    I2C_Disp_Addr = 0x27,       // I2C address of LCD display

    DisplayTime = 7,            // time to display before turning off

    GpsTimeout = 30000,         // timeout on no gps activity
};

enum ServoPos_t {
    // Servo positions
    SrvClose = 60,              // servo position for "box close"
    SrvOpen = 110,              // servo position for "box open"
};

void smartDelay(unsigned long ms = 0);
void init_serial();
void init_gps();
void wait_for_gps(LiquidCrystal_I2C &lcd);
void play_success();
void moveServo(uint8_t const pos);
void lock_box();
void unlock_box();
void turn_off(LiquidCrystal_I2C &lcd);
double get_distance(double latitude, double longitude);
void clear_lcd(LiquidCrystal_I2C &lcd);
void update_lcd(LiquidCrystal_I2C &lcd, char const *line1 = "", char const *line2 = "");
void update_lcd_P(LiquidCrystal_I2C &lcd, char const *line1, char const *line2);
void lcd_off(LiquidCrystal_I2C &lcd);
void update_local_display(LiquidCrystal_I2C &lcd);
void update_target_display(LiquidCrystal_I2C &lcd);
void check_dates(LiquidCrystal_I2C &lcd);
void show_temp_volt(LiquidCrystal_I2C &lcd);
void show_datetime(LiquidCrystal_I2C &lcd);
void check_secret(LiquidCrystal_I2C &lcd);
void check_developer_lock(LiquidCrystal_I2C &lcd);
tm convert_cst(LiquidCrystal_I2C &lcd, int gmt_offset);
void test_all(LiquidCrystal_I2C &lcd);

#endif // UTILITIES_H
