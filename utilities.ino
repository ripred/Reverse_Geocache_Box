/*\
|*| utilities.ino
|*| 
|*| (c) 2022 Trent M. Wyatt.
|*| companion file for Reverse Geocache Box project
|*| 
\*/

#include "utilities.h"

// ----------------------------------------------------------------------
// delay the specified number of milliseconds while
// processing any received serial gps data
// 
void smartDelay(unsigned long ms/* = 0 */) {
    unsigned long start = millis();
    do
    {
        while (ss.available())
            gps.encode(ss.read());
    } while (millis() - start < ms);

} // smartDelay


// ----------------------------------------------------------------------
// open and configure the serial port:
// 
void init_serial() {
#if defined(SERIAL_OUTPUT)
#warning compiling with Serial port enabled.
    Serial.begin(115200);
#endif

} // init_serial


// ----------------------------------------------------------------------
// start software serial talking to the gps module:
// 
void init_gps() {
    ss.begin(4800);

} // init_gps


// ----------------------------------------------------------------------
// Show the initial display text until we get a lock on a satelite:
// 
void wait_for_gps(LiquidCrystal_I2C &lcd) {
    static const char line1[] PROGMEM = "Waiting on GPS..";
    static const char line2[] PROGMEM = "";
    update_lcd_P(lcd, line1, line2);

    while (!gps.location.isValid()) {
        smartDelay();
    }

} // wait_for_gps


// ----------------------------------------------------------------------
// Move the servo to a specified position
void moveServo(ServoPos_t pos) {
    PWMServo servo;         // The servo that locks/unlocks the box
    servo.attach(SrvPin);   // start generating pwm on the pin as an output
    int delta = 0;

    switch (pos) {
        case SrvClose:  // SrvClose := 60
            delta = (SrvOpen > SrvClose) ? -1 : +1;
            for (unsigned newpos = SrvOpen; newpos != SrvClose; newpos += delta) {
                servo.write(newpos);
                smartDelay(16);
            }
            break;

        case SrvOpen:   // SrvOpen := 110
            delta = (SrvOpen > SrvClose) ? +1 : -1;
            for (unsigned newpos = SrvClose; newpos != SrvOpen; newpos += delta) {
                servo.write(newpos);
                smartDelay(16);
            }
            break;
    }

    smartDelay(100);
    servo.detach();         // set the pin as high-z input and stop generating pwm

} // moveServo


// ----------------------------------------------------------------------
// lock the bax
// 
void lock_box() {
    moveServo(SrvClose);
    config_data.setLocked();

} // lock_box


// ----------------------------------------------------------------------
// unlock the box
// 
void unlock_box() {
    moveServo(SrvOpen);
    config_data.setUnlocked();

} // unlock_box


// ----------------------------------------------------------------------
// turn the box off
// 
void turn_off(LiquidCrystal_I2C &lcd) {
    lcd_off(lcd);
    pinMode(PwrPin, OUTPUT);
    digitalWrite(PwrPin, HIGH);

} // turn_off


// ----------------------------------------------------------------------
// get the distance in meters from the current location to
// the specified location
// 
double get_distance(double latitude, double longitude) {
    double distanceMeters = 0.0;

    if (gps.location.isValid()) {
        double const lat = gps.location.lat();
        double const lon = gps.location.lng();

        distanceMeters = TinyGPSPlus::distanceBetween(lat, lon, latitude, longitude);
    }

    return distanceMeters;

} // get_distance


// ----------------------------------------------------------------------
// update the lcd display with the two lines of text from SRAM
// 
void update_lcd(LiquidCrystal_I2C &lcd, char const *line1/* = "" */, char const *line2/* = "" */) {
#if defined(SERIAL_OUTPUT)
    Serial.print(F("\n"));
    Serial.println(line1);
    Serial.println(line2);
#endif

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    smartDelay(200);

} // update_lcd

// ----------------------------------------------------------------------
// update the lcd display with the two lines of text from PROGMEM flash memory
// 
void update_lcd_P(LiquidCrystal_I2C &lcd, char const *line1, char const *line2) {
    char data1[32], data2[32];
    strncpy_P(data1, line1, sizeof(data1));
    strncpy_P(data2, line2, sizeof(data2));
    update_lcd(lcd, data1, data2);

} // update_lcd_P


// ----------------------------------------------------------------------
// turn the lcd display off
// 
void lcd_off(LiquidCrystal_I2C &lcd) {
    lcd.noDisplay();
    smartDelay(200);

} // lcd_off


// ----------------------------------------------------------------------
// display the current latitude and longitude. for debugging/testing
// 
void update_local_display(LiquidCrystal_I2C &lcd) {
    while (!gps.location.isValid()) {
        smartDelay(100);
        if (millis() >= 30000 && gps.charsProcessed() < 10) {
            static const char line1[] PROGMEM = "No GPS data.";
            static const char line2[] PROGMEM = "check wiring.";
            update_lcd_P(lcd, line1, line2);
            smartDelay(DisplayTime * 1000);
            turn_off(lcd);

            return;
        }
    }

//    double next_lat = config_data.get_target_lat(0);
//    double next_lon = config_data.get_target_lon(0);

    double cur_lat = gps.location.lat();
    double cur_lon = gps.location.lng();

    char buff1[64], buff2[64], tmp[16];
    dtostrf(cur_lat, 14, 7, tmp);
    strcpy(buff1, tmp);
    dtostrf(cur_lon, 14, 7, tmp);
    strcpy(buff2, tmp);
    update_lcd(lcd, buff1, buff2);
    smartDelay(DisplayTime * 1000);    

} // update_local_display


// ----------------------------------------------------------------------
// Handle arriving at our next target location!
// 
void target_success(LiquidCrystal_I2C &lcd) {
    // We successfully reached the target!
    static const char line1[] PROGMEM = "Congratulations!";

    // remove the current target from the list of targets
    config_data.remove_target(0);
    
    // see if this is the last target
    if (config_data.get_num_targets() == 0) {
        static const char line2[] PROGMEM = " Box Unlocked!";
        update_lcd_P(lcd, line1, line2);
        unlock_box();
    }
    else {
        static const char line2[] PROGMEM = " More Targets!! ";
        update_lcd_P(lcd, line1, line2);
    }

    config_data.save();

    // play "success" music!
    play_success();
}


// ----------------------------------------------------------------------
// update the display with the distance to the target and number of tries
// remaining. Handles arriving at target(s), reducing the number of tries,
// and possibly unlocking the box and play a song.
// 
void update_target_display(LiquidCrystal_I2C &lcd) {

    // Use Clock Mode if all targets have been solved.
    if (config_data.get_num_targets() == 0) {
        // we have unlocked all of the targets.
        show_datetime(lcd);
        smartDelay(DisplayTime * 1000);
        turn_off(lcd);

        return;
    }

    // See if we are out of tries
    int tries = config_data.get_num_tries();
    if (tries == 0) {
        static const char line1[] PROGMEM = "  You are out";
        static const char line2[] PROGMEM = "   of tries!";
        update_lcd_P(lcd, line1, line2);

        // *make the user wait or something to default things again (*not included here)

        // we just reset things here by default so there is no penalty
        config_data.init();
        config_data.set_default_tries(DefaultTries);
        config_data.save();

        smartDelay(DisplayTime * 1000);
        turn_off(lcd);

        return;
    }

    // wait for gps location to be available
    // alerts for problems in the wiring after 30 second timeout
    while (!gps.location.isValid()) {
        smartDelay(100);
        if (millis() >= GpsTimeout && gps.charsProcessed() < 10) {
            static const char line1[] PROGMEM = "No GPS data.";
            static const char line2[] PROGMEM = "check wiring.";
            update_lcd_P(lcd, line1, line2);
            smartDelay(DisplayTime * 1000);
            turn_off(lcd);

            return;
        }
    }

    // We have GPS now so our location, date, and time are correct

    double target_lat = config_data.get_target_lat(0);
    double target_lon = config_data.get_target_lon(0);

    double distanceMeters = get_distance(target_lat, target_lon);
    double distanceKm = distanceMeters / 1000.0;
    double distanceMiles = distanceKm * 0.6214;

    // round off to two decimal places
    distanceMiles = double(uint32_t(distanceMiles * 100)) / 100.0;

    if (distanceMiles <= 0.01) {
        target_success(lcd);
        turn_off(lcd);

        return;
    }

    config_data.reduce_tries();

#if defined(DEVELOPER_MODE)
#warning compiling for DEVELOPER_MODE.
#else

    config_data.save();

#endif

    char line1[32] = "", line2[32] = "";
    char buf[32];

    dtostrf(distanceMiles, 7, 2, buf);
    strcpy(line1, buf);
    static char const miles[] PROGMEM = " miles.";
    strcat_P(line1, miles);

    static char const fmtP[] PROGMEM = "%d tries left.";
    char fmt[16];
    strcpy_P(fmt, fmtP);
    sprintf(buf, fmt, tries);
    strcpy(line2, buf);

    update_lcd(lcd, line1, line2);

    smartDelay(DisplayTime * 1000);

    turn_off(lcd);

} // update_target_display


// ----------------------------------------------------------------------
// check for any special dates and times
// play a tune and display a message for any
// 
void check_dates(LiquidCrystal_I2C &lcd) {
    while (!gps.date.isValid()) 
    {
        smartDelay();
    }

    int num = config_data.get_num_alerts();
    for (int i=0; i < num; i++) {
        datetime_t dt;
        char msg[2][17];
        if (config_data.get_alert(i, dt, msg[0], msg[1])) {
            if ( dt.get_year() == gps.date.year()
              && dt.get_month() == gps.date.month()
              && dt.get_day() == gps.date.day()

            // include time in check if desired:
              //&& dt.get_hours() == gps.time.hour()
              //&& dt.get_minutes() == gps.time.minute()
              //&& dt.get_seconds() == gps.time.second()
            ) {
                update_lcd(lcd, msg[0], msg[1]);
                play_success();
                //smartDelay(DisplayTime * 1000);
            }
        }
    }

} // check_dates


// ----------------------------------------------------------------------
// display the Vin voltage and processor temperature
// 
void show_temp_volt(LiquidCrystal_I2C &lcd) {
    char line1[32], line2[32], buf[8];

    // calculate Vcc in millivolts:
    signed long millivolts = readVcc();

    static char const fmt1[] PROGMEM = "volts: ";
    strcpy_P(line1, fmt1);

    dtostrf(millivolts/1000.0, 5, 2, buf);
    strcat(line1, buf);

    static char const fmt2[] PROGMEM = "temp: ";
    strcpy_P(line2, fmt2);

    bool bCelcius = false;

    // Read the processor temperature in celcius:
    double temp = temperature();

    // Convert to fahrenheit if desired:
    temp = bCelcius ? temp : (temp * 2.0) + 30.0;

    // Format string of 6 total digits (incl '.') with 2 decimal place
    dtostrf(temp, 6, 2, buf);
    strcat(line2, buf);

    update_lcd(lcd, line1, line2);

} //  show_temp_volt


// ----------------------------------------------------------------------
// show the current date and time
// used in clock mode
// 
void show_datetime(LiquidCrystal_I2C &lcd) {
    char line1[32], line2[32];
    static char const month[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    tm dt = convert_cst(lcd, -6);

    sprintf(line1, "%s %d, %d", month[dt.tm_mon], dt.tm_mday, dt.tm_year + 1900);
    sprintf(line2, "%02d:%02d:%02d", dt.tm_hour, dt.tm_min, dt.tm_sec);
    update_lcd(lcd, line1, line2);

} // show_datetime


// ----------------------------------------------------------------------
// check for the secret backdoor to open the box
// and access any special menus or features
// 
void check_secret(LiquidCrystal_I2C &lcd) {
    // set up one pin as an INPUT that defaults to HIGH. set up the other pin as an OUTPUT:
    pinMode(SecretPin1, INPUT_PULLUP);
    pinMode(SecretPin2, OUTPUT);

    // check to see if the input pin repeatedly follows the output pin:
    int i = 0;
    for (; i < 7; ++i) {
        digitalWrite(SecretPin2, LOW);
        if (digitalRead(SecretPin1))
            break;

        // so far it looks like the pins might be connected. test further.
        digitalWrite(SecretPin2, HIGH);
        if (!digitalRead(SecretPin1))
            break;
    }

    // return the pins to their original high impedence INPUT mode
    pinMode(SecretPin1, INPUT);
    pinMode(SecretPin2, INPUT);

    if (7 != i) {
        // the pins do not appear to be connected so just return and do nothing
        return;
    }
        
    // the pins are connected together!

    static const char line1[] PROGMEM = "-=   Secret   =-";
    static const char line2[] PROGMEM = "-=  Backdoor  =-";
    update_lcd_P(lcd, line1, line2);

    unlock_box();
    config_data.save();

    smartDelay(2000);

    // ----------------------------------------------------------------------
    // wait until we get a valid satellite lock:
    wait_for_gps(lcd);

    update_local_display(lcd);
    play_success();

    turn_off(lcd);
}


// ----------------------------------------------------------------------
// see if the box needs locking and if so
// display a countdown and then lock
// 
void check_developer_lock(LiquidCrystal_I2C &lcd) {
#if defined(DEVELOPER_MODE)
    static char const line1[] PROGMEM = "  Lock Stopped  ";
    static char const line2[] PROGMEM = " DEVELOPER_MODE ";
    update_lcd_P(lcd, line1, line2);
    smartDelay(2000);
    return;
#endif

    if (config_data.get_num_targets() == 0) {
        static char const line1[] PROGMEM = "  Lock Stopped  ";
        static char const line2[] PROGMEM = "   No Targets   ";
        update_lcd_P(lcd, line1, line2);
        smartDelay(2000);
        return;
    }

    if (config_data.isLocked()) {
        return;
    }

    for (int i=10; i >= 0; i--) {
        char line1[32]="", line2[32]="";
        static char const fmt1[] PROGMEM = " Dev Lock Mode  ";
        strcpy_P(line1, fmt1);
        sprintf(line2, "Locking in: %2d..", i);
        update_lcd(lcd, line1, line2);
        smartDelay(850);
    }

    lock_box();

    config_data.setLocked();
    config_data.save();
}


// ----------------------------------------------------------------------
// convert gps satellite time from gmt to some other time zone
// 
tm convert_cst(LiquidCrystal_I2C &lcd, int gmt_offset) {
    wait_for_gps(lcd);
    while (!gps.date.isValid() || !gps.time.isValid()) {
        smartDelay();
    }

    tm t; // std C time and date struct

    t.tm_sec = gps.time.second();
    t.tm_min = gps.time.minute();
    t.tm_hour = gps.time.hour();
    t.tm_mday = gps.date.day();
    t.tm_mon = gps.date.month() - 1;
    t.tm_year = gps.date.year() - 1900;
    t.tm_wday = (t.tm_mday + 3) % 7;
    t.tm_yday = 0;
    t.tm_isdst = 0;

//    char *str_asc = asctime(&t);
//    Serial.println(str_asc);

    // convert from GMT to CST
    t.tm_hour += gmt_offset;

    if (t.tm_hour < 0) {
        t.tm_hour += 24;
        t.tm_mday--;

        if (t.tm_mday <= 0) {
            int feb = t.tm_year % 4 == 0 ? 29 : 28;
            int days[12] = { 
                31, feb,  31,  30, 
                31,  30,  31,  31, 
                30,  31,  30,  31 };
            t.tm_mday = days[t.tm_mon];
            t.tm_wday = (t.tm_mday + 1) % 7;
            t.tm_mon--;
            if (t.tm_mon < 0) {
                t.tm_mon = 11;
                t.tm_year--;
            }
        }

        t.tm_wday--;
        if (t.tm_wday < 0) {
            t.tm_wday = 6;
        }
    }

//    str_asc = asctime(&t);
//    Serial.println(str_asc);

    return t;

} // convert_cst


// ----------------------------------------------------------------------
// test all of the features of the box
// 
void test_all(LiquidCrystal_I2C &lcd) {
    if (!config_data.load()) {
        static const char line1[] PROGMEM = "EEPROM load fail";
        static const char line2[] PROGMEM = "";
        update_lcd_P(lcd, line1, line2);
    }
    else {
        static const char line1[] PROGMEM = "Hello, world!";
        static const char line2[] PROGMEM = "line 2";
        update_lcd_P(lcd, line1, line2);
    }

    update_local_display(lcd);
    smartDelay(DisplayTime * 1000);

    check_dates(lcd);

    show_datetime(lcd);
    smartDelay(2000);

    show_temp_volt(lcd);
    smartDelay(2000);

    uint32_t timer = millis() + DisplayTime * 1000;
    while (millis() < timer) {
        lock_box();
        smartDelay(1000);
        
        unlock_box();
        smartDelay(1000);
    }

    unlock_box();
    
    play_success();

    turn_off(lcd);

} // test_all
