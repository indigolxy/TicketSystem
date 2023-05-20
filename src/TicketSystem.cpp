#include "TicketSystem.h"

void TicketSystem::CheckOrder(const WaitingOrder &order, SeatsDay &seats) {
    for (int i = order.leave_index; i < order.arrive_index; ++i) {
        if (seats.seats[i] < order.num) return;
    }
    // 能够补票
    for (int i = order.leave_index; i < order.arrive_index; ++i) {
        seats.seats[i] -= order.num; // 完成补票
    }
    // 修改order并删除WaitingOrder
    Order tmp = order_map.FindModify({order.user_name, order.time_stamp}, false).second;
    wait_list.remove({{tmp.train_id, tmp.date}, order.time_stamp});
    tmp.status = 1; // success
    order_map.FindModify({order.user_name, order.time_stamp}, true, tmp);
}

int TicketSystem::BuyTicket(const String<UserNameMAXLEN> &u, const String<TrainIDMAXLEN> &id, int d, const String<StaionMAXLEN> &f, const String<StaionMAXLEN> &t, int n, bool can_wait, int time_stamp) {
    if (!user_system.CheckUser(u)) return -1;
    const std::pair<bool, TrainInfo> &target_train = train_system.train_id_info_map.FindModify(id, false);
    if (!target_train.first || !target_train.second.released) return -1;

    const TrainInfo &train = target_train.second;
    if (n > train.seat_num) return -1; // 购买票数超过总座位数，购票失败
    int leave_index = 0, arrive_index = 0;
    for (int i = 1; i <= train.station_num; ++i) {
        if (f == train.stations[i]) {
            leave_index = i;
        }
        else if (t == train.stations[i]) {
            arrive_index = i;
            break;
        }
    }
    if (arrive_index == 0 || leave_index == 0 || arrive_index <= leave_index) return -1;

    d -= train.leaving_times[leave_index] / MinADay;
    if (d < train.sale_date_start || d > train.sale_date_end) return -1;

    Order order(train, d, n, leave_index, arrive_index, time_stamp);
    SeatsDay seats = train_system.seats_day_file.ReadPage(order.seats_day);
    bool seats_are_enough = true;
    for (int i = leave_index; i < arrive_index; ++i) {
        if (seats.seats[i] < n) {
            seats_are_enough = false;
            break;
        }
    }

    if (seats_are_enough) {
        order.status = 1;
        for (int i = leave_index; i < arrive_index; ++i) {
            seats.seats[i] -= n;
        }
        train_system.seats_day_file.WritePage(seats, order.seats_day); // 修改回文件
    }
    else if (!can_wait) {
        return -1;
    }
    else {
        order.status = 0;
        WaitingOrder waiting_order(leave_index, arrive_index, n, u, time_stamp);
        wait_list.insert({{train.train_id, d}, time_stamp}, waiting_order);
    }
    order_map.insert({u, time_stamp}, order);

    if (order.status == 1) return order.price * order.num;
    return 0;
}

std::pair<bool, sjtu::vector<Order>> TicketSystem::QueryOrder(const String<UserNameMAXLEN> &u) {
    if (!user_system.CheckUser(u)) return {false, {}};
    return {true, order_map.find({u, 0}, OrderMapCmp)};
}

bool TicketSystem::RefundTicket(const String<UserNameMAXLEN> &u, int n) {
    if (!user_system.CheckUser(u)) return false;
    const sjtu::vector<Order> &orders = order_map.find({u, 0}, OrderMapCmp);
    int index = orders.size() - n;
    if (index < 0) return false;

    Order order = orders[index];
    if (order.status == -1) return false; // refunded: 什么都不做
    if (order.status == 0) { // pending 在waitinglist中删除，修改order状态
        wait_list.remove({{order.train_id, order.date}, order.time_stamp});
        order.status = -1; // refunded
        order_map.FindModify({u, order.time_stamp}, true, order);
        return true;
    }

    // order.status == 1(success)
    SeatsDay seats = train_system.seats_day_file.ReadPage(order.seats_day);
    for (int i = order.leave_index; i < order.arrive_index; ++i) {
        seats.seats[i] += order.num; // 退回座位
    }

    order.status = -1;
    order_map.FindModify({u, order.time_stamp}, true, order);

    // 按顺序尝试补票
    sjtu::vector<WaitingOrder> waiting_orders = wait_list.find({{order.train_id, order.date}, {}}, WaitListCmp);
    for (int i = 0; i < waiting_orders.size(); ++i) {
        CheckOrder(waiting_orders[i], seats);
    }

    train_system.seats_day_file.WritePage(seats, order.seats_day);
    return true;
}

void TicketSystem::AcceptMsg(const std::string &src, int time_stamp) {
    sjtu::vector<std::string> res = Command::GetTokens(src, ' ');
    // 第一个是time_stamp
    std::cout << res[0] << ' ';
    // 第二个是cmd
    std::string cmd = res[1];
    int i = 2;

    if (cmd == "add_user") {
        String<UserNameMAXLEN> c;
        String<UserNameMAXLEN> u;
        String<PassWordMAXLEN> p;
        String<NameMAXLEN> n;
        String<MailAddrMAXLEN> m;
        int g;

        const size_t &res_size = res.size();
        while (i < res_size) {
            if (res[i] == "-c") c = res[++i];
            else if (res[i] == "-u") u = res[++i];
            else if (res[i] == "-p") p = res[++i];
            else if (res[i] == "-n") n = res[++i];
            else if (res[i] == "-m") m = res[++i];
            else if (res[i] == "-g") {
                g = Command::StringToInt(res[++i]);
            }
            ++i;
        }

        if (user_system.AddUser(c, u, p, n, m, g)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "login") {
        String<UserNameMAXLEN> u;
        String<PassWordMAXLEN> p;

        const size_t &res_size = res.size();
        while (i < res_size) {
            if (res[i] == "-u") u = res[++i];
            else if (res[i] == "-p") p = res[++i];
            ++i;
        }

        if (user_system.Login(u, p)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "logout") {
        String<UserNameMAXLEN> u(res[++i]);

        if (user_system.Logout(u)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "query_profile") {
        String<UserNameMAXLEN> c;
        String<UserNameMAXLEN> u;

        const size_t &res_size = res.size();
        while (i < res_size) {
            if (res[i] == "-u") u = res[++i];
            else if (res[i] == "-c") c = res[++i];
            ++i;
        }

        const std::pair<bool, UserInfo> &user = user_system.QueryProfile(c, u);
        if (!user.first) std::cout << "-1\n";
        else std::cout << user.second;
    }
    else if (cmd == "modify_profile") {
        String<UserNameMAXLEN> c;
        String<UserNameMAXLEN> u;
        String<PassWordMAXLEN> p;
        String<NameMAXLEN> n;
        String<MailAddrMAXLEN> m;
        int g = -1;

        const size_t &res_size = res.size();
        while (i < res_size) {
            if (res[i] == "-c") c = res[++i];
            else if (res[i] == "-u") u = res[++i];
            else if (res[i] == "-p") p = res[++i];
            else if (res[i] == "-n") n = res[++i];
            else if (res[i] == "-m") m = res[++i];
            else if (res[i] == "-g") {
                g = Command::StringToInt(res[++i]);
            }
            ++i;
        }

        const std::pair<bool, UserInfo> &user = user_system.ModifyProfile(c, u, p, n, m, g);
        if (!user.first) std::cout << "-1\n";
        else std::cout << user.second;
    }
    else if (cmd == "add_train") {
        String<TrainIDMAXLEN> id;
        String<StaionMAXLEN> s[StationNumMAX + 1];
        int p[StationNumMAX + 1] = {0};
        int n = 0, m = 0, d1 = 0, d2 = 0, x = 0;
        int t[StationNumMAX + 1] = {0};
        int o[StationNumMAX + 1] = {0};
        char y;

        const size_t &res_size = res.size();
        while (i < res_size) {
            if (res[i] == "-i") id = res[++i];
            else if (res[i] == "-n")
                n = Command::StringToInt(res[++i]);
            else if (res[i] == "-m")
                m = Command::StringToInt(res[++i]);
            else if (res[i] == "-s") {
                sjtu::vector<std::string> stations = Command::GetTokens(res[++i], '|');
                for (int cnt = 0; cnt < stations.size(); ++cnt) {
                    s[cnt + 1] = stations[cnt];
                }
            }
            else if (res[i] == "-p") {
                sjtu::vector<std::string> prices = Command::GetTokens(res[++i], '|');
                for (int cnt = 0; cnt < prices.size(); ++cnt) {
                    p[cnt + 1] = Command::StringToInt(prices[cnt]);
                }
            }
            else if (res[i] == "-x") {
                x = Command::TimeToInt(res[++i]);
            }
            else if (res[i] == "-t") {
                sjtu::vector<std::string> times = Command::GetTokens(res[++i], '|');
                for (int cnt = 0; cnt < times.size(); ++cnt) {
                    t[cnt + 1] = Command::StringToInt(times[cnt]);
                }
            }
            else if (res[i] == "-o") {
                ++i;
                if (res[i] != "_") {
                    sjtu::vector<std::string> overs = Command::GetTokens(res[i], '|');
                    for (int cnt = 0; cnt < overs.size(); ++cnt) {
                        o[cnt + 2] = Command::StringToInt(overs[cnt]);
                    }
                }
            }
            else if (res[i] == "-d") {
                sjtu::vector<std::string> dates = Command::GetTokens(res[++i], '|');
                d1 = Command::DateToInt(dates[0]);
                d2 = Command::DateToInt(dates[1]);
            }
            else if (res[i] == "-y") {
                y = res[++i][0];
            }
            ++i;
        }

        if (train_system.AddTrain(id, n, m, s, p, x, t, o, d1, d2, y)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "delete_train") {
        String<TrainIDMAXLEN> id(res[++i]);
        if (train_system.DeleteTrain(id)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "release_train") {
        String<TrainIDMAXLEN> id(res[++i]);
        if (train_system.ReleaseTrain(id)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "query_train") {
        String<TrainIDMAXLEN> id;
        int d = 0;
        const size_t &res_size = res.size();
        while (i < res_size)  {
            if (res[i] == "-i") id = res[++i];
            else if (res[i] == "-d")
                d = Command::DateToInt(res[++i]);
            ++i;
        }

        if (d == -1) {
            std::cout << "-1\n";
        }
        else {
            const std::pair<std::pair<int, TrainInfo>, SeatsDay> &tmp = train_system.QueryTrain(id, d);
            if (tmp.first.first == -1) std::cout << "-1\n";
            else {
                std::cout << tmp.first.second.PrintTrain(tmp.second, d);
            }
        }
    }
    else if (cmd == "query_ticket") {
        String<StaionMAXLEN> s;
        String<StaionMAXLEN> t;
        int d;
        bool p = true;

        const size_t &res_size = res.size();
        while (i < res_size) {
            if (res[i] == "-s") s = res[++i];
            else if (res[i] == "-t") t = res[++i];
            else if (res[i] == "-d") {
                d = Command::DateToInt(res[++i]);
            }
            else if (res[i] == "-p") {
                if (res[++i] == "cost") p = false;
            }
            ++i;
        }

        if (d == -1) {
            std::cout << "0\n";
        }
        else {
            const sjtu::vector<Ticket> &tmp = train_system.QueryTicket(d, s, t, p);
            const size_t &tmp_size = tmp.size();
            std::cout << tmp_size << '\n';
            for (int cnt = 0; cnt < tmp_size; ++cnt) {
                std::cout << tmp[cnt] << '\n';
            }
        }
    }
    else if (cmd == "query_transfer") {
        String<StaionMAXLEN> s;
        String<StaionMAXLEN> t;
        int d;
        bool p = true;

        const size_t &res_size = res.size();
        while (i < res_size) {
            if (res[i] == "-s") s = res[++i];
            else if (res[i] == "-t") t = res[++i];
            else if (res[i] == "-d") {
                d = Command::DateToInt(res[++i]);
            }
            else if (res[i] == "-p") {
                if (res[++i] == "cost") p = false;
            }
            ++i;
        }

        if (d == -1) {
            std::cout << "0\n";
        }
        else {
            const std::pair<bool, std::pair<Ticket, Ticket>> &tmp = train_system.QueryTransfer(d, s, t, p);
            if (!tmp.first) std::cout << "0\n";
            else std::cout << tmp.second.first << '\n' << tmp.second.second << '\n';
        }
    }
    else if (cmd == "buy_ticket") {
        String<UserNameMAXLEN> u;
        String<TrainIDMAXLEN> id;
        int d, n;
        String<StaionMAXLEN> f;
        String<StaionMAXLEN> t;
        bool q = false;

        const size_t &res_size = res.size();
        while (i < res_size) {
            if (res[i] == "-u") u = res[++i];
            else if (res[i] == "-i") id = res[++i];
            else if (res[i] == "-d") {
                d = Command::DateToInt(res[++i]);
            }
            else if (res[i] == "-n") {
                n = Command::StringToInt(res[++i]);
            }
            else if (res[i] == "-f") {
                f = res[++i];
            }
            else if (res[i] == "-t") {
                t = res[++i];
            }
            else if (res[i] == "-q") {
                if (res[++i] == "true") q = true;
            }
            ++i;
        }

        if (d == -1) std::cout << "-1\n";
        else {
            int tmp = BuyTicket(u, id, d, f, t, n, q, time_stamp);
            if (tmp != 0) std::cout << tmp << '\n';
            else std::cout << "queue\n";
        }
    }
    else if (cmd == "query_order") {
        String<UserNameMAXLEN> u(res[++i]);

        const std::pair<bool, sjtu::vector<Order>> &tmp = QueryOrder(u);
        if (!tmp.first) std::cout << "-1\n";
        else {
            const size_t &tmp_size = tmp.second.size();
            std::cout << tmp_size << '\n';
            for (int cnt = tmp_size - 1 ; cnt >= 0; --cnt) {
                std::cout << tmp.second[cnt] << '\n';
            }
        }
    }
    else if (cmd == "refund_ticket") {
        String<UserNameMAXLEN> u;
        int n = 1;

        const size_t &res_size = res.size();
        while (i < res_size) {
            if (res[i] == "-u") u = res[++i];
            else if (res[i] == "-n") {
                n = Command::StringToInt(res[++i]);
            }
            ++i;
        }

        if (RefundTicket(u, n)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
}

TicketSystem::TicketSystem(const std::string &ticket_system, const std::string &train_system, const std::string &user_system) :
order_map(ticket_system + "1", ticket_system + "2", ticket_system + "3"), wait_list(ticket_system + "4", ticket_system + "5", ticket_system + "6"),
train_system(train_system), user_system(user_system) {}

void TicketSystem::Exit() {
    user_system.user_map.traverse(UserSystem::Operation);
}