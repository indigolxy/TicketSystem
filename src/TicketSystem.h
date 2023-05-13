
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
    void CheckOrder(const WaitingOrder &order, SeatsDay &seats, SeatsWaitingListDay &list, int index) {
        for (int i = order.leave_index; i < order.arrive_index; ++i) {
            if (seats.seats[i] < order.num) return;
        }
        for (int i = order.leave_index; i < order.arrive_index; ++i) {
            seats.seats[i] -= order.num;
        }

        Order tmp = order_map.FindModify({order.user_name, order.no}, false).second;
        tmp.status = 1;
        order_map.FindModify({order.user_name, order.no}, true, tmp);

        for (int i = index; i < list.max_index; ++i) {
            list.waiting_orders[i] = list.waiting_orders[i + 1];
        }
        --list.max_index;
    }

    std::pair<std::pair<std::string, std::string>, sjtu::vector<std::string>> GetTokens(const std::string &s);

    void StringToChar(char *x, const std::string &y) {
        for (int i = 0;i < y.length();++i) {
            x[i] = y[i];
        }
    }

    int StringToInt(const std::string &x) {
        int ans = 0;
        for (char i : x) {
            ans *= 10;
            ans += i - '0';
        }
        return ans;
    }

public:
    TicketSystem(const std::string &order_map, const std::string &train_system, const std::string &user_system);

    int BuyTicket(const char *u, const char *id, int d, const char *f, const char *t, int n, bool can_wait);

    std::pair<bool, sjtu::vector<Order>> QueryOrder(const char *u);

    bool RefundTicket(const char *u, int n);

    void AcceptMsg(const std::string &s);
};

#endif //TICKETSYSTEM_TICKETSYSTEM_H
