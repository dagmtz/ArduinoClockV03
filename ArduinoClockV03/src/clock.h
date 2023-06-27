#ifndef CLOCK_H_
#define CLOCK_H_

#define STARTUP_DELAY_MS 50
#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define HOURS_PER_DAY 24
#define DAYS_PER_MONTH_MAX 31
#define MONTHS_PER_YEAR 12
#define DEBOUNCE_THRESHOLD_MS 20
#define PRESS_THRESHOLD_MS 300
#define REPEAT_THRESHOLD_MS 66

#define BUTTON_SETUP_BIT B00000100
#define BUTTON_PLUS_BIT B00000010
#define BUTTON_MINUS_BIT B00000001
#define SETUP_BUTTONS_MASK B00000111

typedef enum
{
    MINUS,
    NO_SIGN,
    PLUS
} sign_t;

typedef enum
{
    NO_UNIT,
    SECONDS,
    MINUTES,
    HOURS,
    DAYS,
    MONTHS,
    YEARS
} clock_units_t;

typedef enum
{
    SUNDAY,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY
} weekdays_t;

typedef enum
{
    JANUARY = 1,
    FEBRUARY,
    MARCH,
    APRIL,
    MAY,
    JUNE,
    JULY,
    AUGUST,
    SEPTEMBER,
    OCTOBER,
    NOVEMBER,
    DECEMBER
} months_t;

typedef struct
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} time_hhmmss_t;

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
} date_yyyymmdd_t;

typedef enum
{
    STOP,
    RUNNING,
    SETUP
} clock_state_t;

typedef enum
{
    NOT_PRESSED,
    DEBOUNCE,
    SINGLE,
    HOLD,
    WAIT
} button_press_state_t;

typedef struct
{
    uint8_t currentButtonReading;
    uint8_t lastButtonReading;
    uint8_t currentButtonSelected;
    button_press_state_t currentButtonState;
    button_press_state_t lastButtonState;
    clock_units_t currentUnit;
    clock_units_t nextUnit;
    unsigned long lastPressTime;
} settings_control_t;

typedef struct
{
    clock_state_t clockState;
    time_hhmmss_t time;
    date_yyyymmdd_t date;
    uint8_t weekday;
    uint8_t daysInCurrentMonth;
    char timeString[9];
    char dateString[11];
} clock_control_t;

void time_to_BCD(const time_hhmmss_t, uint8_t *);

void date_to_BCD(const date_yyyymmdd_t, uint8_t *);

void clock_to_serial(clock_control_t *);

void update_led_displays(uint8_t *);

void read_buttons(settings_control_t *);

void update_settings(clock_control_t *, settings_control_t *);

void update_clock(clock_control_t *, const clock_units_t, const sign_t, const bool, const bool);

bool is_leap_year(uint16_t);

uint8_t div10(uint8_t);

uint8_t get_weekday(const date_yyyymmdd_t *);

uint8_t days_in_month(uint8_t, uint16_t);

#endif /* CLOCK_H_ */