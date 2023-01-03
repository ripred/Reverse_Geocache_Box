/*\
|*| eeprom_data.h
|*| 
|*| (c) 2022 Trent M. Wyatt.
|*| companion file for Reverse Geocache Box project
|*| 
\*/

#if !defined(EEPROM_DATA)
#define EEPROM_DATA

#include <EEPROM.h>
#include "datetime.h"

// Redefine as needed (and memory allows)
#define   MAX_TARGETS    8

// Redefine as needed (and memory allows)
#define   MAX_DATES      8

// The persistent values in EEPROM
class eeprom_data_t 
{
private:
    static const 
    uint8_t Signature {0xA5};

    uint8_t DefaultTries {50};

    uint8_t signature {0};
    uint8_t dirty {0};
    uint8_t tries {0};
    uint8_t locked {0};
    struct {
        double      lat {0.0};
        double      lon {0.0};
    } targets[MAX_TARGETS];

    struct alert_t {
        datetime_t dt;
        char msg[2][17] { "                ", "                " };
    } dates[MAX_DATES];

public:

    eeprom_data_t()
    {
    }
    
    void init()
    {
        signature = Signature;
        tries = DefaultTries;
        dirty = true;
    }

    bool isValid() const
    {
        return Signature == signature;
    }

    bool isDirty() const
    {
        return dirty;
    }

    bool isLocked() const
    {
        return locked;
    }

    void setLocked()
    {
        if (!locked) {
            locked = true;
            dirty = true;
        }
    }

    void setUnlocked()
    {
        if (locked) {
            locked = false;
            dirty = true;
        }
    }

    // ----------------------------------------------------------------------
    // Targets

    int get_num_targets() const {
        unsigned num = 0;

        for (; num < ARRAYSZ(targets); ++num) {
            if (targets[num].lat == 0.0 && targets[num].lon == 0.0)
                break;
        }
        
        return num;
    }

    bool remove_target(int num) {
        if (num >= get_num_targets())
            return false;

        dirty = true;
        for (; num < get_num_targets()-1; num++) {
            targets[num] = targets[num + 1];
        }

        for (; num < int(ARRAYSZ(targets)); ++num) {
            targets[num].lat = 0.0;
            targets[num].lon = 0.0;
        }
        
        return true;
    }

    double get_target_lat(int cur=0) const
    {
        return targets[cur].lat;
    }
    
    void set_target_lat(double const latitude, int cur)
    {
        if (targets[cur].lat != latitude) {
            targets[cur].lat = latitude;
            dirty = true;
        }
    }

    double get_target_lon(int cur = 0) const
    {
        return targets[cur].lon;
    }
    
    void set_target_lon(double const longitude, int cur)
    {
        if (targets[cur].lon != longitude) {
            targets[cur].lon = longitude;
            dirty = true;
        }
    }

    // ----------------------------------------------------------------------
    // Number of Tries:

    uint8_t get_default_tries() const
    {
        return DefaultTries;
    }

    void set_default_tries(uint8_t t)
    {
        DefaultTries = t;
    }

    uint16_t get_num_tries() const
    {
        return tries;
    }

    void set_tries(uint16_t const t)
    {
        if (tries != t) {
            tries = t;
            dirty = true;
        }
    }

    uint16_t reduce_tries()
    {
        if (tries > 0)
        {
            dirty = true;
            return --tries;
        }

        return 0;
    }

    // ----------------------------------------------------------------------
    // Alerts for special dates/times:

    int get_num_alerts() const
    {
        unsigned num = 0;
        for (; num < ARRAYSZ(dates); num++)
        {
            if (dates[num].dt.get_year() == 0)
                break;
        }

        return num;
    }

    bool get_alert(unsigned i, datetime_t &dt, char msg1[], char msg2[]) const
    {
        if (i < ARRAYSZ(dates))
        {
            dt = dates[i].dt;
            strcpy(msg1, dates[i].msg[0]);
            strcpy(msg2, dates[i].msg[1]);

            return true;
        }

        return false;
    }

    bool add_alert(datetime_t const &dt, char const *msg1, char const *msg2)
    {
        unsigned num = get_num_alerts();
        for (unsigned i = 0; i < num; ++i) {
            alert_t &alert = dates[i];
            if (alert.dt == dt && !strcmp(alert.msg[0], msg1) && !strcmp(alert.msg[1], msg2)) {
                return true;
            }
        }

        if (num >= ARRAYSZ(dates)) {
            return  false;
        }

        alert_t &alert = dates[num];

        alert.dt = dt;
        strcpy(alert.msg[0], msg1);
        strcpy(alert.msg[1], msg2);

        dirty = true;

        return true;
    }

    // ----------------------------------------------------------------------
    // Loading from EEPROM and Saving to EEPROM

    bool load()
    {
        EEPROM.get(0, *this);

        if (!isValid())
        {
            init();
            save();
        }

        dirty = false;

        return isValid();
    }

    void save()
    {
//        if (dirty) {
            dirty = false;
            EEPROM.put(0, *this);
//        }
    }

};

#endif // EEPROM_DATA
