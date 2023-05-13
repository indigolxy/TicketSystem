
#ifndef TICKETSYSTEM_TRAINSYSTEM_H
#define TICKETSYSTEM_TRAINSYSTEM_H

#include "UserSystem.h"

constexpr int TrainIDMAXLEN = 20;
constexpr int StationNumMAX = 100;
constexpr int StaionMAXLEN = 10 * 3;
constexpr int WaitingNumMAX = 100;
constexpr int DateNumMAX = 92;
constexpr int MinADay = 24 * 60;

class TrainSystem;
class TrainInfo;

class SeatsDay {
    friend class TrainSystem;
    friend class Ticket;
    friend class TicketSystem;

private:
    int seats[StationNumMAX + 1] = {0};

public:
    SeatsDay() = default;
    SeatsDay(int seat_num, int station_num) {
        for (int i = 1; i <= station_num; ++i) {
            seats[i] = seat_num;
        }
    }
    void Initialize(int seat_num, int station_num) {
        for (int i = 1; i <= station_num; ++i) {
            seats[i] = seat_num;
        }
    }
};

class WaitingOrder {
    friend class TicketSystem;
private:
    int leave_index;
    int arrive_index;
    int num;
    char user_name[UserNameMAXLEN + 5];
    int no;

public:
    WaitingOrder() = default;
    WaitingOrder(int leave, int arrive, int n, const char *u, int noo) : leave_index(leave), arrive_index(arrive), num(n), no(noo) {
        strcpy(user_name, u);
    }
};

class SeatsWaitingListDay {
    friend class TicketSystem;
private:
    WaitingOrder waiting_orders[WaitingNumMAX + 1];
    int max_index = -1; // 目前优先级最低的waiting_order的下标

public:
    SeatsWaitingListDay() : max_index(-1),waiting_orders() {}
};

class TrainStation {
    friend class TrainSystem;
private:
    char train_id[TrainIDMAXLEN + 5];
    int index; // 在bpt中对应station在该车次中的下标

public:
    TrainStation() = default;
    TrainStation(const char *id, int ind) : index(ind) {
        strcpy(train_id, id);
    }
};

class Ticket {
    friend class TrainSystem;
private:
    char train_id[TrainIDMAXLEN + 5];
    char leave_station_name[StaionMAXLEN + 5];
    int leave_station_time; // leaving_time
    char arrive_station_name[StaionMAXLEN + 5];
    int arrive_station_time; // arriving_time
    int seat;
    int time;
    int cost;
    int start_date; // 发车日期

public:
    Ticket() = default;
    Ticket(const TrainInfo &src, int leave_index, int arrive_index, SeatsDay seats, int start_d);
};

class TrainInfo {
    friend class TrainSystem;
    friend class TrainStation;
    friend class Ticket;
    friend class TicketSystem;
    friend class Order;

private:
    char train_id[TrainIDMAXLEN + 5];
    int station_num;
    int seat_num;
    bool released;
    char type;
    int sale_date_start;
    int sale_date_end;
    char stations[StationNumMAX + 1][StaionMAXLEN + 5];
    int prices[StationNumMAX + 1];
    int arriving_times[StationNumMAX + 1];
    int leaving_times[StationNumMAX + 1];
    Ptr seats;
    Ptr seats_waiting_lists;

public:
    TrainInfo() = default;
};

class TrainSystem {
    friend class TicketSystem;
private:
    BPlusTree<std::pair<String<StaionMAXLEN>, String<TrainIDMAXLEN>>, TrainStation> station_train_map;
    BPlusTree<String<TrainIDMAXLEN>, TrainInfo> train_id_info_map;
    FileSystem<SeatsDay> seats_day_file;
    FileSystem<SeatsWaitingListDay> seats_waiting_list_day_file;

    static bool TrainStationCmp(const std::pair<String<StaionMAXLEN>, String<TrainIDMAXLEN>> &a, const std::pair<String<StaionMAXLEN>, String<TrainIDMAXLEN>> &b) {
        return strcmp(a.first.data, b.first.data) < 0;
    }

    static bool TicketCmp(const Ticket &a, const Ticket &b, bool key_is_time);

    static bool TicketPairCmp(const std::pair<Ticket, Ticket> &a, const std::pair<Ticket, Ticket> &b, bool key_is_time);

    std::pair<bool, std::pair<Ticket, Ticket>> CheckTrainStations(int leave_index, int arrive_index, bool key_is_time, int leave_date,
                                                                  const TrainInfo &leave_train, const TrainInfo &arrive_train);

public:
    TrainSystem(const std::string &train_system);

    bool AddTrain(const char *id, int n, int m, const char *s[StationNumMAX + 1], int p[StationNumMAX + 1],
                  int x, int t[StationNumMAX + 1], int o[StationNumMAX + 1], int d1, int d2, char y);

    bool DeleteTrain(const char *id);

    bool ReleaseTrain(const char *id);

    // * 查询失败: -1  unreleased(seats == seat_num): 0 released(all parameters are used): 1
    std::pair<std::pair<int, TrainInfo>, SeatsDay> QueryTrain(const char *id, int d);

    sjtu::vector<Ticket> QueryTicket(int d, const char *s, const char *t, bool key_is_time);

    std::pair<bool, std::pair<Ticket, Ticket>> QueryTransfer(int d, const char *s, const char *t, bool key_is_time);

    // * 检查train是否已经发布，若失败返回false
    std::pair<bool, TrainInfo> GetTrain(const char *id);
};

#endif //TICKETSYSTEM_TRAINSYSTEM_H
