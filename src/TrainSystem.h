
#ifndef TICKETSYSTEM_TRAINSYSTEM_H
#define TICKETSYSTEM_TRAINSYSTEM_H

#include "UserSystem.h"
#include "utils/DateTime.h"
#include "utils/Command.h"

constexpr int TrainIDMAXLEN = 20;
constexpr int StationNumMAX = 100;
constexpr int StaionMAXLEN = 10 * 3;
constexpr int DateNumMAX = 92;
constexpr int MinADay = 24 * 60;

class TrainSystem;
class TrainInfo;
class TrainStation;

class SeatsDay {
    friend class TrainSystem;
    friend class Ticket;
    friend class TicketSystem;
    friend class TrainInfo;

private:
    int seats[StationNumMAX + 1] = {0};

public:
    SeatsDay() = default;
    SeatsDay(int seat_num, int station_num) {
        for (int i = 1; i < station_num; ++i) {
            seats[i] = seat_num;
        }
    }
    void Initialize(int seat_num, int station_num) {
        for (int i = 1; i < station_num; ++i) {
            seats[i] = seat_num;
        }
    }
};

class Ticket {
    friend class TrainSystem;
private:
    String<TrainIDMAXLEN> train_id;
    String<StaionMAXLEN> leave_station_name;
    int leave_station_time; // leaving_time
    String<StaionMAXLEN> arrive_station_name;
    int arrive_station_time; // arriving_time
    int seat;
    int time;
    int cost;
    int start_date; // 发车日期

    void SetSeat(const SeatsDay &seats, int leave_index, int arrive_index) {
        seat = seats.seats[arrive_index - 1];
        for (int k = leave_index; k < arrive_index; ++k) {
            if (seats.seats[k] < seat) seat = seats.seats[k];
        }
    }

public:
    Ticket() = default;

    Ticket(const TrainStation &leave, const TrainStation &arrive, const SeatsDay &seats, int start_d);
    Ticket(const TrainInfo &src, int leave_index, int arrive_index, int start_d);

    Ticket(const Ticket &other);

    friend std::ostream &operator<<(std::ostream &os, const Ticket &obj) {
        os << obj.train_id << ' ' << obj.leave_station_name << ' ';
        int leave_date = obj.start_date + obj.leave_station_time / MinADay;
        int leave_time = obj.leave_station_time % MinADay;
        os << Command::IntToDate(leave_date) << ' ' << Command::IntToTime(leave_time);
        os << " -> ";
        os << obj.arrive_station_name << ' ';
        int arrive_date = obj.start_date + obj.arrive_station_time / MinADay;
        int arrive_time = obj.arrive_station_time % MinADay;
        os << Command::IntToDate(arrive_date) << ' ' << Command::IntToTime(arrive_time);
        os << ' ' << std::to_string(obj.cost) << ' ' << std::to_string(obj.seat);
        return os;
    }
};

class TrainInfo {
    friend class TrainSystem;
    friend class TrainStation;
    friend class Ticket;
    friend class TicketSystem;
    friend class Order;

private:
    String<TrainIDMAXLEN> train_id;
    int station_num;
    int seat_num;
    bool released;
    char type;
    int sale_date_start;
    int sale_date_end;
    String<StaionMAXLEN> stations[StationNumMAX + 1];
    int prices[StationNumMAX + 1];
    int arriving_times[StationNumMAX + 1];
    int leaving_times[StationNumMAX + 1];
    Ptr seats;

public:
    TrainInfo() = default;
    std::string PrintTrain(const SeatsDay &seats_day, int start_day) const ;
};

class TrainStation {
    friend class TrainSystem;
    friend class Ticket;
private:
    String<TrainIDMAXLEN> train_id;
    int index; // 在bpt中对应station在该车次中的下标
    int leaving_time;
    int arriving_time;
    int price;
    int sale_date_start;
    int sale_date_end;
    String<StaionMAXLEN> station_name;
    Ptr seats;

public:
    TrainStation() = default;
    TrainStation(const TrainInfo &src, int index_) : index(index_), sale_date_start(src.sale_date_start), sale_date_end(src.sale_date_end), seats(src.seats),
    train_id(src.train_id), station_name(src.stations[index]){
        leaving_time = src.leaving_times[index];
        arriving_time = src.arriving_times[index];
        price = src.prices[index];
    }
};

constexpr int StationTrainMapT = ((4096 * 2 - 5) / (StaionMAXLEN + 1 + TrainIDMAXLEN + 1 + 4) - 2) / 2;
constexpr int StationTrainMapL = ((4096 * 2 - 8) / (StaionMAXLEN + 1 + TrainIDMAXLEN + 1 + sizeof(TrainStation)) - 2) / 2;
constexpr int StationTrainMapBN = 64;
constexpr int StationTrainMapBL = 64; // 1024K
constexpr int TrainIDInfoMapT = ((4096 * 4 - 5) / (TrainIDMAXLEN + 1 + 4) - 2) / 2;
constexpr int TrainIDInfoMapL = ((4096 * 16 - 8) / (TrainIDMAXLEN + 1 + sizeof(TrainInfo)) - 2) / 2;
constexpr int TrainIDInfoMapBN = 42;
constexpr int TrainIDInfoMapBL = 42; // 3360K

class TrainSystem {
    friend class TicketSystem;
private:
    BPlusTree<std::pair<String<StaionMAXLEN>, String<TrainIDMAXLEN>>, TrainStation, StationTrainMapT, StationTrainMapL, StationTrainMapBN, StationTrainMapBL> station_train_map;
    BPlusTree<String<TrainIDMAXLEN>, TrainInfo, TrainIDInfoMapT, TrainIDInfoMapL, TrainIDInfoMapBN, TrainIDInfoMapBL> train_id_info_map;
    FileSystem<SeatsDay, 1000> seats_day_file;

    static bool TrainStationCmp(const std::pair<String<StaionMAXLEN>, String<TrainIDMAXLEN>> &a, const std::pair<String<StaionMAXLEN>, String<TrainIDMAXLEN>> &b) {
        return a.first < b.first;
    }

    static bool TicketCmp(const Ticket &a, const Ticket &b, bool key_is_time);

    static bool TicketPairCmp(const std::pair<Ticket, Ticket> &a, const std::pair<Ticket, Ticket> &b, bool key_is_time);

    std::pair<bool, std::pair<Ticket, Ticket>> CheckTrainStations(int leave_index, int arrive_index, bool key_is_time, int leave_date,const TrainInfo &leave_train, const TrainInfo &arrive_train);

    static void Qsort(sjtu::vector<Ticket> &ans, int l, int r, bool key_is_time);

    static int Qdivide(sjtu::vector<Ticket> &ans, int l, int r, bool key_is_time);

public:
    TrainSystem(const std::string &train_system);

    bool AddTrain(const String<TrainIDMAXLEN> &id, int n, int m, const String<StaionMAXLEN> s[StationNumMAX + 1], int p[StationNumMAX + 1],
                  int x, int t[StationNumMAX + 1], int o[StationNumMAX + 1], int d1, int d2, char y);

    bool DeleteTrain(const String<TrainIDMAXLEN> &id);

    bool ReleaseTrain(const String<TrainIDMAXLEN> &id);

    // * 查询失败: -1  unreleased(seats == seat_num): 0 released(all parameters are used): 1
    std::pair<std::pair<int, TrainInfo>, SeatsDay> QueryTrain(const String<TrainIDMAXLEN> &id, int d);

    sjtu::vector<Ticket> QueryTicket(int d, const String<StaionMAXLEN> &s, const String<StaionMAXLEN> &t, bool key_is_time);

    std::pair<bool, std::pair<Ticket, Ticket>> QueryTransfer(int d, const String<StaionMAXLEN> &s, const String<StaionMAXLEN> &t, bool key_is_time);
};

#endif //TICKETSYSTEM_TRAINSYSTEM_H
