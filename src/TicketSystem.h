
#ifndef TICKETSYSTEM_TICKETSYSTEM_H
#define TICKETSYSTEM_TICKETSYSTEM_H

#include "TrainSystem.h"

class Order {
    char train_id[TrainIDMAXLEN + 5];
    int date; // 发车日期
    int numm;
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
};

class TicketSystem {
private:
    BPlusTree<std::pair<String<UserNameMAXLEN>, int>, Order> order_map;
    UserSystem user_system;
    TrainSystem train_system;

public:
    int BuyTicket(const char *u, const char *id, int d, const char *f, const char *t, int n, bool can_wait);

    std::pair<bool, sjtu::vector<Order>> QueryOrder(const char *u);

    bool RefundTicket(const char *u, int n);
};

#endif //TICKETSYSTEM_TICKETSYSTEM_H
