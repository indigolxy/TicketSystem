
#ifndef TICKETSYSTEM_DATETIME_H
#define TICKETSYSTEM_DATETIME_H

class Date {
    int month;
    int day;

public:
    Date(int mm, int dd) : month(mm), day(dd) {}
    Date(int d) {
        if (d < 30) {
            month = 6;
            day = d + 1;
        }
        else if (d < 61) {
            month = 7;
            day = d - 29;
        }
        else if (d < 92){
            month = 8;
            day = d - 60;
        }
        else {
            month = 9;
            day = d - 91;
        }
    }
    static Date IntToDate(int d) {
        if (d < 30) return {6, d + 1};
        if (d < 61) return {7, d - 29};
        if (d < 92) return {8, d - 60};
        return {9, d - 91};
    }
    int DateToInt() const {
        if (month == 6) return day - 1;
        if (month == 7) return day + 29;
        if (month == 8) return day + 60;
        if (month == 9) return day + 91;
        return -1;
    }
    std::string DateToString() const {
        std::string ans;
        if (month < 10) ans += "0";
        ans += std::to_string(month);
        ans += "-";
        if (day < 10) ans += "0";
        ans += std::to_string(day);
        return ans;
    }
};

class Time {
    int hour;
    int minute;

public:
    Time() = default;
    Time(int hh, int mm) : hour(hh), minute(mm) {}
    Time(int t) : hour(t / 60), minute(t % 60) {}
    static Time IntToTime(int t) {
        return {t / 60, t % 60};
    }
    int TimeToInt() {
        return hour * 60 + minute;
    }
    std::string TimeToString() const {
        std::string ans;
        if (hour < 10) ans += "0";
        ans += std::to_string(hour);
        ans += ":";
        if (minute < 10) ans += "0";
        ans += std::to_string(minute);
        return ans;
    }
};

#endif //TICKETSYSTEM_DATETIME_H
