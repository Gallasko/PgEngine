#pragma once

class GameHour
{
public:
    GameHour() : day(0), hour(0), minute(0) { }
    GameHour(unsigned int hour, unsigned int minute) : day(0), hour(hour), minute(minute) { }
    GameHour(unsigned int day, unsigned int hour, unsigned int minute) : day(day), hour(hour), minute(minute) { }

    inline bool operator==(const GameHour& rhs) const { return day == rhs.day && hour == rhs.hour && minute == rhs.minute; }
    inline bool operator!=(const GameHour& rhs) const { return !(*this == rhs); }

    inline bool operator<(const GameHour& rhs) const { return day < rhs.day ? true : day > rhs.day ? false : hour < rhs.hour ? true : hour > rhs.hour ? false : minute < rhs.minute; }
    
    inline bool operator<=(const GameHour& rhs) const { return *this < rhs || *this == rhs; }
    inline bool operator>=(const GameHour& rhs) const { return !(*this < rhs); }
    inline bool operator>(const GameHour& rhs) const { return !(*this <= rhs); }

private:
    unsigned int day;
    unsigned int hour;
    unsigned int minute;
};