
#ifndef TICKETSYSTEM_TICKETSYSTEM_H
#define TICKETSYSTEM_TICKETSYSTEM_H

#include "TrainSystem.h"

class Order {
    friend class TicketSystem;
private:
    char train_id[TrainIDMAXLEN + 5];
    int date; // 发车日期
    int num;
    int price;
    int status; // 1 success，0 pending，-1 refunded

    int leave_index;
    char leave_station[StaionMAXLEN + 5];
    int leave_time;
    int arrive_index;
    char arrive_station[StaionMAXLEN + 5];
    int arrive_time;

    Ptr seats_day; // 该车次该发车日期对应的SeatsDay的读写位置
    Ptr seats_waiting_list_day; // 同为“该发车日期”date对应的位置

public:
    Order() = default;
    Order(const TrainInfo &src, int d, int n, int leave, int arrive) : date(d), num(n), leave_index(leave), arrive_index(arrive) {
        strcpy(train_id, src.train_id);
        strcpy(leave_station, src.stations[leave]);
        strcpy(arrive_station, src.stations[arrive]);
        leave_time = src.leaving_times[leave];
        arrive_time = src.arriving_times[arrive];
        seats_day = src.seats + d * sizeof(SeatsDay);
        seats_waiting_list_day = src.seats_waiting_lists + d * sizeof(SeatsWaitingListDay);
        price = 0;
        for (int i = leave; i < arrive; ++i) {
            price += src.prices[i];
        }
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
        int arrive_date = obj.date + obj.arrive_time / MinADay;
        int av_time = obj.arrive_time % MinADay;
        os << Command::IntToDate(arrive_date) << ' ' << Command::IntToTime(av_time);
        os << ' ' << obj.price << ' ' << obj.num;
    }
};

class TicketSystem {
private:
    BPlusTree<std::pair<String<UserNameMAXLEN>, int>, Order> order_map;
    UserSystem user_system;
    TrainSystem train_system;

    static bool OrderMapCmp(const std::pair<String<UserNameMAXLEN>, int> &a, const std::pair<String<UserNameMAXLEN>, int> &b) {
        return strcmp(a.first.data, b.first.data) < 0;
    }

    // * 引用传参修改list和seats
    void CheckOrder(const WaitingOrder &order, SeatsDay &seats, SeatsWaitingListDay &list, int index);

public:
    TicketSystem(const std::string &order_map, const std::string &train_system, const std::string &user_system);

    int BuyTicket(const char *u, const char *id, int d, const char *f, const char *t, int n, bool can_wait);

    std::pair<bool, sjtu::vector<Order>> QueryOrder(const char *u);

    bool RefundTicket(const char *u, int n);

    void AcceptMsg(const std::string &s);
};

#endif //TICKETSYSTEM_TICKETSYSTEM_H
