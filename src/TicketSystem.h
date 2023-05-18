
#ifndef TICKETSYSTEM_TICKETSYSTEM_H
#define TICKETSYSTEM_TICKETSYSTEM_H

#include "TrainSystem.h"

class Order {
    friend class TicketSystem;
private:
    char train_id[TrainIDMAXLEN + 1];
    int date; // 发车日期
    int num;
    int price;
    int status; // 1 success，-1 refunded, 0 pending

    int leave_index;
    char leave_station[StaionMAXLEN + 1];
    int leave_time;
    int arrive_index;
    char arrive_station[StaionMAXLEN + 1];
    int arrive_time;
    int time_stamp;

    Ptr seats_day; // 该车次该发车日期对应的SeatsDay的读写位置

public:
    Order() = default;
    // * 未设置status
    Order(const TrainInfo &src, int d, int n, int leave, int arrive, int time) : date(d), num(n), leave_index(leave), arrive_index(arrive), time_stamp(time) {
        strcpy(train_id, src.train_id);
        strcpy(leave_station, src.stations[leave]);
        strcpy(arrive_station, src.stations[arrive]);
        leave_time = src.leaving_times[leave];
        arrive_time = src.arriving_times[arrive];
        seats_day = src.seats + d * sizeof(SeatsDay);
        price = src.prices[arrive] - src.prices[leave];
    }
    friend std::ostream &operator<<(std::ostream &os, const Order &obj) {
        if (obj.status == 1) os << "[success] ";
        else if (obj.status == 0) os << "[pending] ";
        else os << "[refunded] ";

        os << obj.train_id << ' ' << obj.leave_station << ' ';
        int leave_date = obj.date + obj.leave_time / MinADay;
        int lv_time = obj.leave_time % MinADay;
        os << Command::IntToDate(leave_date) << ' ' << Command::IntToTime(lv_time);
        os << " -> ";
        os << obj.arrive_station << ' ';
        int arrive_date = obj.date + obj.arrive_time / MinADay;
        int av_time = obj.arrive_time % MinADay;
        os << Command::IntToDate(arrive_date) << ' ' << Command::IntToTime(av_time);
        os << ' ' << obj.price << ' ' << obj.num;
        return os;
    }
};

class WaitingOrder {
    friend class TicketSystem;
private:
    int leave_index;
    int arrive_index;
    int num;
    char user_name[UserNameMAXLEN + 1];
    int time_stamp;

public:
    WaitingOrder() = default;
    WaitingOrder(int leave, int arrive, int n, const char *u, int time) : leave_index(leave), arrive_index(arrive), num(n), time_stamp(time) {
        strcpy(user_name, u);
    }
};

constexpr int OrderMapT = ((4096 - 5) / (4 + UserNameMAXLEN + 1 + 4) - 2) / 2;
constexpr int OrderMapL = ((4096 * 2 - 8) / (4 + UserNameMAXLEN + 1 + sizeof(Order)) - 2) / 2;
constexpr int OrderMapBN = 40;
constexpr int OrderMapBL = 40;
constexpr int WaitListT = ((4096 - 5) / (4 + TrainIDMAXLEN + 1 + 4 + 4) - 2) / 2;
constexpr int WaitListL = ((4096 - 8) / (4 + TrainIDMAXLEN + 1 + 4 + sizeof(WaitingOrder)) - 2) / 2;
constexpr int WaitListBN = 30;
constexpr int WaitListBL = 30;

class TicketSystem {
private:
    BPlusTree<std::pair<String<UserNameMAXLEN>, int>, Order, OrderMapT, OrderMapL, OrderMapBN, OrderMapBL> order_map;
    BPlusTree<std::pair<std::pair<String<TrainIDMAXLEN>, int>, int>, WaitingOrder, WaitListT, WaitListL, WaitListBN, WaitListBL> wait_list;
    UserSystem user_system;
    TrainSystem train_system;

    static bool OrderMapCmp(const std::pair<String<UserNameMAXLEN>, int> &a, const std::pair<String<UserNameMAXLEN>, int> &b) {
        return strcmp(a.first.data, b.first.data) < 0;
    }

    static bool WaitListCmp(const std::pair<std::pair<String<TrainIDMAXLEN>, int>, int> &a, const std::pair<std::pair<String<TrainIDMAXLEN>, int>, int> &b) {
        if (strcmp(a.first.first.data, b.first.first.data) < 0) return true;
        if (strcmp(a.first.first.data, b.first.first.data) > 0) return false;
        return a.first.second < b.first.second;
    }

    // * 引用传参修改seats
    void CheckOrder(const WaitingOrder &order, SeatsDay &seats);

public:
    TicketSystem(const std::string &order_map, const std::string &train_system, const std::string &user_system);

    int BuyTicket(const char *u, const char *id, int d, const char *f, const char *t, int n, bool can_wait, int time_stamp);

    std::pair<bool, sjtu::vector<Order>> QueryOrder(const char *u);

    bool RefundTicket(const char *u, int n);

    void AcceptMsg(const std::string &s, int time_stamp);

    void Exit();
};

#endif //TICKETSYSTEM_TICKETSYSTEM_H
