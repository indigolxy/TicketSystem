#include "TrainSystem.h"
#include <algorithm>
#include <vector>

bool TrainSystem::AddTrain(const char *id, int n, int m, const char **s, int *p, int x, int *t, int *o, int d1, int d2,
                           char y) {
    TrainInfo target_train;
    strcpy(target_train.train_id, id);
    target_train.station_num = n;
    target_train.seat_num = m;
    target_train.released = false;
    target_train.type = y;
    target_train.sale_date_start = d1;
    target_train.sale_date_end = d2;
    target_train.seats = -1;
    target_train.seats_waiting_lists = -1;
    int time_track = x; // == start_time
    for (int i = 1; i <= n; ++i) {
        strcpy(target_train.stations[i], s[i]);
        if (i < n) target_train.prices[i] = p[i];
        if (i > 1) {
            target_train.arriving_times[i] = time_track;
            time_track += o[i];
        }
        if (i < n) {
            target_train.leaving_times[i] = time_track;
            time_track += t[i];
        }
    }
    return train_id_info_map.insert(id, target_train);
}

bool TrainSystem::DeleteTrain(const char *id) {
    std::pair<bool, TrainInfo> target_train = train_id_info_map.FindModify(id, false);
    if (!target_train.first || target_train.second.released) return false; // 不存在 || 车次已发布
    train_id_info_map.remove(id);
    return true;
}

bool TrainSystem::ReleaseTrain(const char *id) {
    std::pair<bool, TrainInfo> target = train_id_info_map.FindModify(id, false);
    if (!target.first) return false; // 车次不存在
    TrainInfo target_train = target.second;
    if (target_train.released) return false; // 车次已发布
    target_train.released = true;

    int st = target_train.sale_date_start;
    int ed = target_train.sale_date_end;
    int seatn = target_train.seat_num;
    int stationn = target_train.station_num;

    Ptr tmp = -1;
    if (st == 0)
        tmp = seats_day_file.WritePage(SeatsDay(seatn, stationn), -1);
    else
        tmp = seats_day_file.WritePage(SeatsDay(), -1);
    target_train.seats = tmp;
    for (int i = 1; i < DateNumMAX; ++i) {
        tmp += sizeof(SeatsDay);
        if (st <= i && i <= ed) seats_day_file.WritePage(SeatsDay(seatn, stationn), tmp);
        else seats_day_file.WritePage(SeatsDay(), tmp);
    }

    tmp = target_train.seats_waiting_lists = seats_waiting_list_day_file.WritePage(SeatsWaitingListDay(), -1);
    for (int i = 1; i < DateNumMAX; ++i) {
        tmp += sizeof(SeatsWaitingListDay);
        seats_waiting_list_day_file.WritePage(SeatsWaitingListDay(), tmp);
    }

    train_id_info_map.FindModify(id, true, target_train);

    for (int i = 1; i <= stationn; ++i) {
        TrainStation ts(id, i);
        station_train_map.insert({target_train.stations[i], id}, ts);
    }

    return true;
}

std::pair<std::pair<int, TrainInfo>, SeatsDay> TrainSystem::QueryTrain(const char *id, int d) {
    std::pair<bool, TrainInfo> target = train_id_info_map.FindModify(id, false);
    if (!target.first) return {{-1, TrainInfo()}, SeatsDay()}; // 车次不存在
    TrainInfo target_train = target.second;
    if (target_train.sale_date_start > d || target_train.sale_date_end < d) return {{-1, TrainInfo()}, SeatsDay()}; // 发车日期不存在
    if (target_train.released) {
        SeatsDay target_seats_day = seats_day_file.ReadPage(target_train.seats);
        return {{1, target_train}, target_seats_day};
    }
    return {{0, target_train}, SeatsDay()};
}

Ticket::Ticket(const TrainInfo &src, int leave_index, int arrive_index, SeatsDay seats, int start_d) {
    strcpy(train_id, src.train_id);
    strcpy(leave_station_name, src.stations[leave_index]);
    strcpy(arrive_station_name, src.stations[arrive_index]);
    start_date = start_d;

    leave_station_time = src.leaving_times[leave_index];
    arrive_station_time = src.arriving_times[arrive_index];
    time = arrive_station_time - leave_station_time;

    cost = 0;
    seat = seats.seats[arrive_index];
    for (int k = leave_index; k < arrive_index; ++k) {
        cost += src.prices[k];
        if (seats.seats[k] < seat) seat = seats.seats[k];
    }
}

bool TrainSystem::TicketCmp(const Ticket &a, const Ticket &b, bool key_is_time) {
    if (key_is_time) {
        if (a.time < b.time) return true;
        if (a.time > b.time) return false;
    }
    else {
        if (a.cost < b.cost) return true;
        if (a.cost > b.cost) return false;
    }
    return strcmp(a.train_id, b.train_id) < 0;
}

bool TrainSystem::TicketPairCmp(const std::pair<Ticket, Ticket> &a, const std::pair<Ticket, Ticket> &b,
                                bool key_is_time) {
    int a_cost = a.first.cost + a.second.cost;
    int b_cost = b.first.cost + b.second.cost;

    int a_time = a.second.start_date * MinADay + a.second.arrive_station_time - a.first.start_date * MinADay - a.first.leave_station_time;
    int b_time = b.second.start_date * MinADay + b.second.arrive_station_time - b.first.start_date * MinADay - b.first.leave_station_time;

    if (key_is_time) {
        if (a_time < b_time) return true;
        if (a_time > b_time) return false;
        if (a_cost < b_cost) return true;
        if (a_cost > b_cost) return false;
    }
    else {
        if (a_cost < b_cost) return true;
        if (a_cost > b_cost) return false;
        if (a_time < b_time) return true;
        if (a_time > b_time) return false;
    }
    int sign = strcmp(a.first.train_id, b.first.train_id);
    if (sign < 0) return true;
    if (sign > 0) return false;
    return strcmp(a.second.train_id, b.second.train_id) < 0;
}

sjtu::vector<Ticket> TrainSystem::QueryTicket(int d, const char *s, const char *t, bool key_is_time) {
    sjtu::vector<TrainStation> leave= station_train_map.find({s, nullptr}, TrainStationCmp);
    sjtu::vector<TrainStation> arrive= station_train_map.find({s, nullptr}, TrainStationCmp);
    if (leave.empty() || arrive.empty()) return {};

    int i = 0, j = 0;
    sjtu::vector<Ticket> ans;
    while (i < leave.size() || j < arrive.size()) {
        int sign = strcmp(leave[i].train_id, arrive[i].train_id);
        if (sign < 0) {
            if (i < leave.size() - 1) ++i;
            else break;
        }
        else if (sign > 0) {
            if (j < arrive.size() - 1) ++j;
            else break;
        }
        else {
            int leave_index = leave[i].index;
            int arrive_index = arrive[j].index;
            if (leave_index < arrive_index) {
                TrainInfo target_train = train_id_info_map.FindModify(leave[i].train_id, false).second;

                d -= target_train.leaving_times[leave[i].index] / MinADay; // 处理成发车日期
                if (target_train.sale_date_start <= d && d <= target_train.sale_date_end) {
                    SeatsDay seats = seats_day_file.ReadPage(target_train.seats + d * sizeof(SeatsDay));
                    Ticket tmp(target_train, leave_index, arrive_index, seats, d);
                    ans.push_back(tmp);
                }
            }

            if (i < leave.size() - 1 && j < arrive.size() - 1) {
                ++i;
                ++j;
            }
            else break;
        }
    }

    if (ans.size() <= 1) return ans;

    // todo sort
//    std::vector<Ticket> std_ans;
//    for (int i = 0; i < ans.size(); ++i) {
//        std_ans.push_back(ans[i]);
//    }
    std::sort(ans.begin(), ans.end(), TicketCmp);

    return ans;
}

std::pair<bool, std::pair<Ticket, Ticket>> TrainSystem::QueryTransfer(int d, const char *s, const char *t, bool key_is_time) {
    sjtu::vector<TrainStation> leave= station_train_map.find({s, nullptr}, TrainStationCmp);
    sjtu::vector<TrainStation> arrive= station_train_map.find({s, nullptr}, TrainStationCmp);
    std::pair<Ticket, Ticket> ans;
    bool has_ans = false;

    for (int i = 0; i < leave.size(); ++i) {
        TrainInfo leave_train = train_id_info_map.FindModify(leave[i].train_id, false).second;
        // leave_train实际的发车日期
        int leave_date = d - leave_train.leaving_times[leave[i].index] / MinADay;
        if (leave_train.sale_date_start > leave_date || leave_date > leave_train.sale_date_end) {
            continue;
        }

        for (int j = 0; j < arrive.size(); ++j) {
            if (strcmp(leave[i].train_id, arrive[j].train_id) == 0) continue;
            TrainInfo arrive_train = train_id_info_map.FindModify(arrive[j].train_id, false).second;

            std::pair<bool, std::pair<Ticket, Ticket>> tmp = CheckTrainStations(leave[i].index, arrive[i].index, key_is_time, leave_date, leave_train, arrive_train);
            if (tmp.first) {
                if (!has_ans) {
                    ans = tmp.second;
                    has_ans = true;
                }
                else {
                    if (TicketPairCmp(tmp.second, ans, key_is_time)) ans = tmp.second;
                }
            }
        }
    }
    return {has_ans, ans};
}

std::pair<bool, std::pair<Ticket, Ticket>> TrainSystem::CheckTrainStations(int leave_index, int arrive_index, bool key_is_time, int leave_date,
                                                                           const TrainInfo &leave_train, const TrainInfo &arrive_train) {
    bool has_ans = false;
    std::pair<Ticket, Ticket> ans;
    for (int lv = leave_index + 1; lv <= leave_train.station_num; ++lv) {
        for (int av = arrive_index - 1; av > 0; --av) {
            if (strcmp(leave_train.stations[lv], arrive_train.stations[av]) == 0) {
                // 到达换乘站的时间: t = leave_date + leave_train.arriving_times[lv]
                // arrive_train发车日期date + arrive_train.leaving_times[av] >= t
                // date*MinADay >= leave_date*MinADay + (leave_train.arriving_times[lv] - arrive_train.leaving_times[av])
                int arrive_date_min = 0, arrive_date = 0;
                if (leave_train.arriving_times[lv] > arrive_train.leaving_times[av])
                    arrive_date_min = leave_date + (leave_train.arriving_times[lv] - arrive_train.leaving_times[av] - 1) / MinADay + 1;
                else
                    arrive_date_min = leave_date + (leave_train.arriving_times[lv] - arrive_train.leaving_times[av]) / MinADay;

                if (arrive_date_min > arrive_train.sale_date_end) continue; // 无法换乘，去看下一站
                if (arrive_date_min >= arrive_train.sale_date_start) arrive_date = arrive_date_min;
                else arrive_date = arrive_train.sale_date_start;

                // 如果上述检查没有问题，即可以换乘
                SeatsDay leave_seats = seats_day_file.ReadPage(leave_train.seats + leave_date * sizeof(SeatsDay));
                SeatsDay arrive_seats = seats_day_file.ReadPage(arrive_train.seats + arrive_date * sizeof(SeatsDay));
                std::pair<Ticket, Ticket> tmp = {{leave_train, leave_index, lv, leave_seats, leave_date}, {arrive_train, av, arrive_index, arrive_seats, arrive_date}};

                if (!has_ans) {
                    ans = tmp;
                    has_ans = true;
                }
                else {
                    if (TicketPairCmp(tmp, ans, key_is_time)) ans = tmp;
                }
            }
        }
    }

    return {has_ans, ans};
}