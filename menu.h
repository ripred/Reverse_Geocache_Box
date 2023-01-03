/*\
|*| menu.h
|*| 
|*| (c) 2022 Trent M. Wyatt.
|*| companion file for Reverse Geocache Box project
|*| 
\*/

#if !defined(MENU_H_INC)
#define MENU_H_INC

struct lcd_menu_t;
enum entry_t : uint8_t
{
    FUNC = 0, 
    MENU = 1, 
     INT = 2
};

struct variant_t
{
    union {
        void      (*func)();
        lcd_menu_t *menu;
        int         ival;
    }       value;
    entry_t type;

    variant_t const & operator = (void (*func)()) 
    {
        (*this).value.func = func;
        type = FUNC;
        return *this;
    }

    variant_t const & operator = (lcd_menu_t *menu) 
    {
        (*this).value.menu = menu;
        type = MENU;
        return *this;
    }

    variant_t const & operator = (int ival) 
    {
        (*this).value.ival = ival;
        type = INT;
        return *this;
    }

}; // variant_t

struct menu_t 
{
    char      txt[17];
    variant_t value;
    int       min;
    int       max;

    menu_t()
    {
        txt[0] = '\0';
        value = 0;
        min = 0;
        max = 0;
    }
};

struct lcd_menu_t 
{
    menu_t  menu[2];
    uint8_t cur : 1;

    lcd_menu_t()
    {
        for (menu_t &entry : menu) {
            entry.txt[0] = '\0';
            entry.value = 0;
            entry.min = 0;
            entry.max = 0;
        }
    }

    lcd_menu_t(char *msg1, void(*func1)(), char *msg2, void(*func2)())
    {
        strncpy(menu[0].txt, msg1, sizeof(menu[0].txt));
        menu[0].value = func1;
        strncpy(menu[1].txt, msg2, sizeof(menu[1].txt));
        menu[1].value = func2;
    }

    lcd_menu_t(char *msg1, lcd_menu_t *menu1, char *msg2, lcd_menu_t *menu2)
    {
        strncpy(menu[0].txt, msg1, sizeof(menu[0].txt));
        menu[0].value = menu1;
        strncpy(menu[1].txt, msg2, sizeof(menu[1].txt));
        menu[1].value = menu2;
    }

    lcd_menu_t(char const *msg1, void(*func1)(), char const *msg2, void(*func2)())
    {
        strncpy(menu[0].txt, msg1, sizeof(menu[0].txt));
        menu[0].value = func1;
        strncpy(menu[1].txt, msg2, sizeof(menu[1].txt));
        menu[1].value = func2;
    }

    lcd_menu_t(char const *msg1, lcd_menu_t *menu1, char const *msg2, lcd_menu_t *menu2)
    {
        strncpy(menu[0].txt, msg1, sizeof(menu[0].txt));
        menu[0].value = menu1;
        strncpy(menu[1].txt, msg2, sizeof(menu[1].txt));
        menu[1].value = menu2;
    }

    int next() 
    {
        return cur = !cur;

    }

    lcd_menu_t &exec() {
        switch (menu[cur].value.type) {
            case FUNC:
                if (menu[cur].value.value.func != nullptr) {
                    menu[cur].value.value.func();
                }
                break;

            case MENU:
                if (menu[cur].value.value.menu != nullptr) {
                    *this = *(menu[cur].value.value.menu);
                }
                break;

            case INT:
                break;
        }

        return *this;
    }

    // 
};

#endif // MENU_H_INC
