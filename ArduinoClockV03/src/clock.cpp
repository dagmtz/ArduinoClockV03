#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>
#include <Wire.h>

#include "clock.h"

#define SERIAL_DEBUG_LEVEL 0

void time_to_BCD(const time_hhmmss_t timeBuffer, uint8_t *bcdBuffer)
{
    bcdBuffer[0] = div10(timeBuffer.hours);
    bcdBuffer[1] = timeBuffer.hours - (bcdBuffer[0] * 10);
    bcdBuffer[2] = div10(timeBuffer.minutes);
    bcdBuffer[3] = timeBuffer.minutes - (bcdBuffer[2] * 10);
    bcdBuffer[4] = div10(timeBuffer.seconds);
    bcdBuffer[5] = timeBuffer.seconds - (bcdBuffer[4] * 10);
}

void date_to_BCD(const date_yyyymmdd_t dateBuffer, uint8_t *bcdBuffer)
{

    bcdBuffer[0] = div10(div10(div10(dateBuffer.year)));
    bcdBuffer[1] = div10(div10(dateBuffer.year - (bcdBuffer[0] * 1000)));
    bcdBuffer[2] = div10(dateBuffer.year - (bcdBuffer[0] * 1000) - (bcdBuffer[1] * 100));
    bcdBuffer[3] = dateBuffer.year - ((bcdBuffer[0] * 1000) - (bcdBuffer[1] * 100) - (bcdBuffer[1] * 10));
    bcdBuffer[5] = div10(dateBuffer.month);
    bcdBuffer[6] = dateBuffer.month - (bcdBuffer[5] * 10);
    bcdBuffer[8] = div10(dateBuffer.day);
    bcdBuffer[9] = dateBuffer.day - (bcdBuffer[8] * 10);
}

void clock_to_serial(clock_control_t *clock)
{
    char buffer[50];
    char weekday[12];
    switch (get_weekday(&clock->date))
    {
    case SUNDAY:
        strcpy(weekday, "SUNDAY   ");
        break;

    case MONDAY:
        strcpy(weekday, "MONDAY   ");
        break;

    case TUESDAY:
        strcpy(weekday, "TUESDAY  ");
        break;

    case WEDNESDAY:
        strcpy(weekday, "WEDNESDAY");
        break;

    case THURSDAY:
        strcpy(weekday, "THURSDAY ");
        break;

    case FRIDAY:
        strcpy(weekday, "FRIDAY   ");
        break;

    case SATURDAY:
        strcpy(weekday, "SATURDAY ");
        break;

    default:
        strcpy(weekday, "         ");
        break;
    }
    sprintf(buffer, "%d-%02d-%02d %s %02d:%02d:%02d", clock->date.year, clock->date.month, clock->date.day, weekday, clock->time.hours, clock->time.minutes, clock->time.seconds);
    Serial.println(buffer);
}

void update_led_displays(uint8_t *time_array)
{
    Wire.beginTransmission(9);
    for (size_t i = 0; i < 6; i++)
    {
        Wire.write(time_array[i]);
    }
    Wire.endTransmission();
}

void read_buttons(settings_control_t *settings)
{
    settings->currentButtonReading = (~PINC) & SETUP_BUTTONS_MASK;

    switch (settings->currentButtonState)
    {
    case NOT_PRESSED:
        if (settings->currentButtonReading != 0)
        {
            settings->lastPressTime = millis();
            settings->lastButtonReading = settings->currentButtonReading;

            switch (settings->currentButtonReading)
            {
            case BUTTON_SETUP_BIT:
                settings->currentButtonSelected = 7;
#if (SERIAL_DEBUG_LEVEL > 0)
                Serial.println("Pressed SETUP");
#endif
                break;

            case BUTTON_PLUS_BIT:
                settings->currentButtonSelected = 6;
#if (SERIAL_DEBUG_LEVEL > 0)
                Serial.println("Pressed PLUS");
#endif
                break;

            case BUTTON_MINUS_BIT:
                settings->currentButtonSelected = 5;
#if (SERIAL_DEBUG_LEVEL > 0)
                Serial.println("Pressed MINUS");
#endif
                break;

            default:
                settings->currentButtonSelected = 0;
                break;
            }

            settings->currentButtonState = DEBOUNCE;
        }
        break;

    case DEBOUNCE:
        if ((settings->currentButtonReading == settings->lastButtonReading) && (settings->currentButtonSelected != 0))
        {
#if (SERIAL_DEBUG_LEVEL > 0)
            Serial.print("Debouncing: ");
            Serial.print(settings->currentButtonReading);
            Serial.println();
#endif
            settings->currentButtonState = WAIT;
            settings->lastButtonState = DEBOUNCE;
        }
        break;

    case SINGLE:
        if ((settings->currentButtonReading == settings->lastButtonReading) && (settings->currentButtonSelected != 0))
        {
#if (SERIAL_DEBUG_LEVEL > 0)
            Serial.print("Single press: ");
            Serial.print(settings->currentButtonReading);
            Serial.println();
#endif
            settings->currentButtonState = WAIT;
            settings->lastButtonState = SINGLE;
            settings->lastPressTime = millis();
        }
        else
        {
#if (SERIAL_DEBUG_LEVEL > 0)
            Serial.println("Aborted hold for ");
            Serial.print(settings->currentButtonReading);
            Serial.println();
#endif
            settings->currentButtonState = NOT_PRESSED;
            settings->lastPressTime = 0;
            settings->lastButtonReading = 0;
            settings->currentButtonSelected = 0;
        }
        break;

    case HOLD:
        if ((settings->currentButtonReading == settings->lastButtonReading) && (settings->currentButtonSelected != 0))
        {
#if (SERIAL_DEBUG_LEVEL > 0)            
            Serial.print("Holding press: ");
            Serial.print(settings->currentButtonReading);
            Serial.println();
#endif            
            settings->currentButtonState = WAIT;
            settings->lastButtonState = HOLD;
            settings->lastPressTime = millis();
        }
        else
        {
#if (SERIAL_DEBUG_LEVEL > 0)            
            Serial.println("Aborted hold for ");
            Serial.print(settings->currentButtonReading);
            Serial.println();
#endif            
            settings->currentButtonState = NOT_PRESSED;
            settings->lastPressTime = 0;
            settings->lastButtonReading = 0;
            settings->currentButtonSelected = 0;
        }
        break;

    case WAIT:
        if ((settings->currentButtonReading == settings->lastButtonReading) && (settings->currentButtonSelected != 0))
        {
#if (SERIAL_DEBUG_LEVEL > 0)            
            Serial.println(millis() - settings->lastPressTime);
            Serial.print("Waiting for ");
#endif            
            switch (settings->lastButtonState)
            {
            case DEBOUNCE:
#if (SERIAL_DEBUG_LEVEL > 0)            
                Serial.print("debounce");
#endif                
                if ((millis() - settings->lastPressTime) > DEBOUNCE_THRESHOLD_MS)
                {
                    settings->currentButtonState = SINGLE;
                    settings->lastButtonState = WAIT;
                }
                break;

            case SINGLE:
#if (SERIAL_DEBUG_LEVEL > 0)            
                Serial.print("single");
#endif                
                if ((millis() - settings->lastPressTime) > PRESS_THRESHOLD_MS)
                {
                    settings->currentButtonState = HOLD;
                    settings->lastButtonState = WAIT;
                }
                break;

            case HOLD:
#if (SERIAL_DEBUG_LEVEL > 0)            
                Serial.print("hold");
#endif                
                if ((millis() - settings->lastPressTime) > REPEAT_THRESHOLD_MS)
                {
                    settings->currentButtonState = HOLD;
                    settings->lastButtonState = WAIT;
                }
                break;

            default:
                break;
            }
#if (SERIAL_DEBUG_LEVEL > 0)            
            Serial.println();
#endif            
        }
        else
        {
#if (SERIAL_DEBUG_LEVEL > 0)            
            Serial.println("Aborted wait");
#endif            
            settings->currentButtonState = NOT_PRESSED;
            settings->lastButtonState = WAIT;
            settings->lastPressTime = 0;
            settings->lastButtonReading = 0;
            settings->currentButtonSelected = 0;
        }
        break;

    default:
        break;
    }
}

void update_settings(clock_control_t *clock, settings_control_t *settings)
{
    switch (clock->clockState)
    {
    case RUNNING:
        switch (settings->currentButtonSelected)
        {
        case 7:
            if (settings->currentButtonState == SINGLE)
            {
                clock->clockState = SETUP;
                Serial.println("SETUP: SECONDS");
                settings->currentUnit = SECONDS;
            }
            break;

        default:
            break;
        }
        break;

    default:
        break;

    case SETUP:
        switch (settings->currentButtonSelected)
        {
        case 7:
            if (settings->currentButtonState == SINGLE)
            {
                switch (settings->currentUnit)
                {
                case SECONDS:
                    Serial.println("SETUP: MINUTES");
                    settings->currentUnit = MINUTES;
                    break;

                case MINUTES:
                    Serial.println("SETUP: HOURS");
                    settings->currentUnit = HOURS;
                    break;

                case HOURS:
                    Serial.println("SETUP: DAYS");
                    settings->currentUnit = DAYS;
                    break;

                case DAYS:
                    Serial.println("SETUP: MONTHS");
                    settings->currentUnit = MONTHS;
                    break;

                case MONTHS:
                    Serial.println("SETUP: YEARS");
                    settings->currentUnit = YEARS;
                    break;

                case YEARS:
                    Serial.println("EXIT SETUP");
                    settings->currentUnit = NO_UNIT;
                    clock->clockState = RUNNING;
                    break;

                default:
                    break;
                }
            }
            break;

        case 6:
            if ((settings->currentButtonState == SINGLE || settings->currentButtonState == HOLD) && (clock->clockState == SETUP))
            {
                update_clock(clock, settings->currentUnit, PLUS, false, true);
            }
            break;

        case 5:
            if ((settings->currentButtonState == SINGLE || settings->currentButtonState == HOLD) && (clock->clockState == SETUP))
            {
                update_clock(clock, settings->currentUnit, MINUS, false, true);
            }
            break;

        default:
            break;
        }
        break;
    }
}

void update_clock(clock_control_t *clockControl, const clock_units_t unit, const sign_t sign, const bool affectNextUnit, const bool printTime)
{
    switch (unit)
    {
    case SECONDS:
        switch (sign)
        {
        case MINUS:
            clockControl->time.seconds++;
            if (clockControl->time.seconds >= SECONDS_PER_MINUTE)
            {
                clockControl->time.seconds = SECONDS_PER_MINUTE - 1;
            }
            break;

        case PLUS:
            clockControl->time.seconds++;
            if (clockControl->time.seconds >= SECONDS_PER_MINUTE)
            {
                clockControl->time.seconds = 0;
                if (affectNextUnit)
                {
                    update_clock(clockControl, MINUTES, PLUS, true, false);
                }
            }
            break;

        default:
            break;
        }
        break;

    case MINUTES:
        switch (sign)
        {
        case MINUS:
            clockControl->time.minutes--;
            if (clockControl->time.minutes >= MINUTES_PER_HOUR)
            {
                clockControl->time.minutes = MINUTES_PER_HOUR - 1;
            }
            break;

        case PLUS:
            clockControl->time.minutes++;
            if (clockControl->time.minutes >= MINUTES_PER_HOUR)
            {
                clockControl->time.minutes = 0;
                if (affectNextUnit)
                {
                    update_clock(clockControl, HOURS, PLUS, true, false);
                }
            }
            break;

        default:
            break;
        }
        break;

    case HOURS:
        switch (sign)
        {
        case MINUS:
            clockControl->time.hours--;
            if (clockControl->time.hours >= HOURS_PER_DAY)
            {
                clockControl->time.hours = HOURS_PER_DAY - 1;
            }
            break;

        case PLUS:
            clockControl->time.hours++;
            if (clockControl->time.hours >= HOURS_PER_DAY)
            {
                clockControl->time.hours = 0;
                if (affectNextUnit)
                {
                    update_clock(clockControl, DAYS, PLUS, true, false);
                }
            }
            break;

        default:
            break;
        }
        break;

    case DAYS:
        switch (sign)
        {
        case MINUS:
            clockControl->date.day--;
            if ((clockControl->date.day == 0) || (clockControl->date.day >= clockControl->daysInCurrentMonth))
            {
                clockControl->date.day = days_in_month(clockControl->date.month, clockControl->date.year);
            }
            break;

        case PLUS:
            clockControl->date.day++;
            if (clockControl->date.day >= clockControl->daysInCurrentMonth)
            {
                clockControl->date.day = 1;
                if (affectNextUnit)
                {
                    update_clock(clockControl, MONTHS, PLUS, true, false);
                }
            }
            break;

        case NO_SIGN:
            if (clockControl->date.day >= clockControl->daysInCurrentMonth)
            {
                clockControl->date.day = days_in_month(clockControl->date.month, clockControl->date.year);
            }
            break;

        default:
            break;
        }
        clockControl->weekday = get_weekday(&clockControl->date);
        break;

    case MONTHS:
        switch (sign)
        {
        case MINUS:
            clockControl->date.month--;
            if ((clockControl->date.month == 0) || (clockControl->date.month >= MONTHS_PER_YEAR))
            {
                clockControl->date.month = MONTHS_PER_YEAR;
            }
            clockControl->daysInCurrentMonth = days_in_month(clockControl->date.month, clockControl->date.year);
            update_clock(clockControl, DAYS, NO_SIGN, false, false);
            break;

        case PLUS:
            clockControl->date.month++;
            if (clockControl->date.month >= MONTHS_PER_YEAR)
            {
                clockControl->date.month = 1;
                if (affectNextUnit)
                {
                    update_clock(clockControl, YEARS, PLUS, true, false);
                }
            }
            clockControl->daysInCurrentMonth = days_in_month(clockControl->date.month, clockControl->date.year);
            update_clock(clockControl, DAYS, NO_SIGN, false, false);
            break;

        default:
            break;
        }
        break;

    case YEARS:
        switch (sign)
        {
        case MINUS:
            clockControl->date.year--;
            if ((clockControl->date.year == 0) || (clockControl->date.year >= 3000))
            {
                clockControl->date.year = 0;
            }
            break;

        case PLUS:
            clockControl->date.year++;
            if (clockControl->date.year >= 3000)
            {
                clockControl->date.year = 3000;
            }
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }

    if (printTime)
    {
        clock_to_serial(clockControl);
    }
}

bool is_leap_year(uint16_t year)
{
    if ((year % 4 == 0) && ((year % 100 != 0) || year % 400 == 0))
    {
        return true;
    }
    return false;
}

uint8_t div10(uint8_t number)
{
    uint8_t q, r;
    q = (number >> 1) + (number >> 2);
    q = q + (q >> 4);
    q = q + (q >> 8);
    q = q >> 3;
    r = number - (((q << 2) + q) << 1);
    return q + (r > 9);
}

uint8_t get_weekday(const date_yyyymmdd_t *date)
{
    uint16_t y = date->year;
    uint8_t m = date->month;
    uint8_t d = date->day;

    return (d += m < 3 ? y-- : y - 2, 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;
}

uint8_t days_in_month(uint8_t month, uint16_t year)
{
    switch (month)
    {
    case 1:
        return 31;
        break;

    case 2:
        if (is_leap_year(year))
        {
            return 29;
        }
        else
        {
            return 28;
        }
        break;

    case 3:
        return 31;
        break;

    case 4:
        return 30;
        break;

    case 5:
        return 31;
        break;

    case 6:
        return 30;
        break;

    case 7:
        return 31;
        break;

    case 8:
        return 31;
        break;

    case 9:
        return 30;
        break;

    case 10:
        return 31;
        break;

    case 11:
        return 30;
        break;

    case 12:
        return 31;
        break;
    }
    return 31;
}
