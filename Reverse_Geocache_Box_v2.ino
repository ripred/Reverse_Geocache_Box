/*\
|*| Reverse_Geocache_Box.ino
|*| main file for Reverse Geocache Box project
|*| 
|*| (c) 2022 Trent M. Wyatt.
|*| 
|*| Features so far:
|*| 
|*| [+] Box only unlocks at a special location.
|*| [+] Written for Arduino Nano but this will work with any other 
|*|     Arduino except ATTiny due to pins. (would work w/o music)
|*| [+] Runs on 2 3.7V rechargeable lipo batteries
|*| [+] 5V LDO (low drop-out) regulator circuit
|*| [+] Uses solid state power button.  Turns itself off.  0.000A used when off.
|*| [+] I2C LCD display
|*| [+] 4800 baud serial GPS module
|*| [+] Metal Gear Digital Servo controls the box lock mechanism
|*| [+] Amplifier module and 2W speaker
|*| [+] Plays any MIDI tune on success using the Playtune library and Miditones
|*|     utility available at https://github.com/LenShustek/miditones.git
|*| [+] Hidden easter egg to unlock or get into internal debugger/setup mode
|*| [+] Displays messages and plays sounds on important dates.
|*| [+] Up to 8 special dates/times with messages can be stored!
|*| [+] Up to 8 sequential targets can be configured!
|*| [+] All configurable target(s), important date(s) and messages, and
|*|     remaining tries are stored in EEPROM.
|*| [+] Detects if EEPROM has never been programmed and automatically
|*|     updates it the first time if needed.
|*| [+] Only modified values are updated in EEPROM - saves write cycles
|*| [+] Displays battery voltage and cpu temperature using the ATMega328's
|*|     internal ADC and diode!
|*| [+] Clock mode
|*| [+] Menu system
|*| 
|*| TODO:
|*| [ ] give the user a direction heading
|*| [ ] menu for user-configurable target(s)
|*| [ ] menu for user-configurable important date(s) and messages
|*| [ ] alarms during clock mode
|*| 
\*/

// see "utilities.h" for pin assignments and change if needed
#include "utilities.h"

// ======================================================================
// Global variables

// The TinyGPSPlus object
TinyGPSPlus      gps;

// The serial connection to the GPS device
SoftwareSerial   ss(RXPin, TXPin);

// The persistent values in EEPROM
eeprom_data_t    config_data;

// ======================================================================

void loop()
{
}

void setup()
{
    init_serial();

    LiquidCrystal_I2C lcd(I2C_Disp_Addr, 16, 2);
    lcd.init();
    lcd.backlight();

    init_gps();

    // ------------------------------------------------------------
    // run the following at least once to update the EEPROM with
    // updated target(s), date(s), and number of remaining retries
#if 0

    // ------------------------------------------------------------
    // set up the new persistent data from the EEPROM:
    config_data.init();
    config_data.save();

    config_data.set_default_tries(DefaultTries);

    // Set any special date(s). Date and time are gmt
    datetime_t alert = datetime_t(2022, 12, 29, 20, 30, 0);
    config_data.add_alert(alert, "Alert", "One");

    // Set at least one target:
    static const double LONDON_LAT = 51.509865, LONDON_LON = -0.118092;
    config_data.set_target_lat(LONDON_LAT, 0);
    config_data.set_target_lon(LONDON_LON, 0);

    unlock_box();

    config_data.save();

    // wait until we get a valid satellite lock:
    wait_for_gps(lcd);

    update_target_display(lcd);
    smartDelay(DisplayTime * 1000);

    turn_off(lcd);

#endif

#if defined(SELF_TEST)
#warning compiling for self test diagnostics.

    // ------------------------------------------------------------
    // wait until we get a valid satellite lock:
    wait_for_gps(lcd);

    // ------------------------------------------------------------
    // run the diagnostic/debugging tests:
    test_all(lcd);

#else

    // ------------------------------------------------------------
    // load the persistent data from the EEPROM:
    config_data.load();

    // ------------------------------------------------------------
    // check for the secret back-door:
    check_secret(lcd);

    // see if we need to lock the box
    check_developer_lock(lcd);

    // ------------------------------------------------------------
    // wait until we get a valid satellite lock:
    wait_for_gps(lcd);

    // ------------------------------------------------------------
    // check for special dates
    check_dates(lcd);

    // ------------------------------------------------------------
    // update the display with the target distance
    // and reduce the number of remaining tries
    update_target_display(lcd);

#endif

} // setup
