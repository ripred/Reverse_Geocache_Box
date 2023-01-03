/*\
|*| datetime.h
|*| 
|*| (c) 2022 Trent M. Wyatt.
|*| companion file for Reverse Geocache Box project
|*| 
\*/

#if !defined(DATETIME_H)
#define DATETIME_H

#include <EEPROM.h>

struct datetime_t {
private:
    uint16_t 
                year : 12,  // 38b 5B for year (0-4095)
               month :  4,  // 26b 4B for month (1-12)
                 day :  5,  // 22b 3B for day (1-31)
               hours :  5,  // 17b 3B for hours (0-23)
             minutes :  6,  // 12b 2B for minutes (0-59)
             seconds :  6;  //  6b 1B for seconds (0-59)

public:
    datetime_t() {
        // Initialize all fields to 0
        year = 0;
        month = 0;
        day = 0;
        hours = 0;
        minutes = 0;
        seconds = 0;
    }

    bool operator == (datetime_t const &r) {
        return r.year == year &&
               r.month == month &&
               r.day == day &&
               r.hours == hours &&
               r.minutes == minutes &&
               r.seconds == seconds;
    }

    bool operator != (datetime_t const &r) {
        return r.year != year ||
               r.month != month ||
               r.day != day ||
               r.hours != hours ||
               r.minutes != minutes ||
               r.seconds != seconds;
    }

    datetime_t(uint16_t y, uint16_t m, uint16_t d, uint16_t hr, uint16_t min, uint16_t sec) {
        year = y;
        month = m;
        day = d;
        hours = hr;
        minutes = min;
        seconds = sec;
    }

    // Set the seconds field
    void set_seconds(unsigned int seconds) {
        if (seconds < 60) {
            seconds = seconds;
        }
    }

    // Get the seconds field
    unsigned int get_seconds() const {
        return seconds;
    }

    // Set the minutes field
    void set_minutes(unsigned int minutes) {
        if (minutes < 60) {
            minutes = minutes;
        }
    }

    // Get the minutes field
    unsigned int get_minutes() const {
        return minutes;
    }

    // Set the hours field
    void set_hours(unsigned int hours) {
        if (hours < 24) {
            hours = hours;
        }
    }

    // Get the hours field
    unsigned int get_hours() const {
        return hours;
    }

    // Set the day field
    void set_day(unsigned int day) {
        if (day > 0 && day < 32) {
            day = day;
        }
    }

    // Get the day field
    unsigned int get_day() const {
        return day;
    }

    // Set the month field
    void set_month(unsigned int month) {
        if (month > 0 && month < 13) {
            month = month;
        }
    }

    // Get the month field
    unsigned int get_month() const {
        return month;
    }

    // Set the year field
    void set_year(unsigned int year) {
        if (year < 4096) {
            year = year;
        }
    }

    // Get the year field
    unsigned int get_year() const {
        return year;
    }

    // Convert a time of day to seconds
    static unsigned int to_seconds(
        unsigned int hr,
        unsigned int min,
        unsigned int sec)
    {
        return (hr * 60 * 60) + (min * 60) + sec;
    }
};

#endif // DATETIME_H
