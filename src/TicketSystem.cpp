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
    Order tmp = order_map.FindModify({order.user_name, order.no}, false).second;
    wait_list.remove({{tmp.train_id, tmp.date}, tmp.status});
    tmp.status = 0; // success
    order_map.FindModify({order.user_name, order.no}, true, tmp);
}

int TicketSystem::BuyTicket(const char *u, const char *id, int d, const char *f, const char *t, int n, bool can_wait) {
    std::pair<int, UserInfo> res = user_system.CheckUserGetOrder(u);
    if (res.first == -1) return -1;

    int no = res.first + 1;
    std::pair<bool, TrainInfo> tmp = train_system.GetTrain(id);
    if (!tmp.first) return -1;

    TrainInfo train = tmp.second;
    int leave_index = 0, arrive_index = 0;
    for (int i = 1; i <= train.station_num; ++i) {
        if (strcmp(f, train.stations[i]) == 0) {
            leave_index = i;
        }
        else if (strcmp(t, train.stations[i]) == 0) {
            arrive_index = i;
            break;
        }
    }
    if (arrive_index == 0 || leave_index == 0 || arrive_index <= leave_index) return -1;

    d -= train.leaving_times[leave_index] / MinADay;
    if (d < train.sale_date_start || d > train.sale_date_end) return -1;

    Order order(train, d, n, leave_index, arrive_index);
    SeatsDay seats = train_system.seats_day_file.ReadPage(order.seats_day);
    bool seats_are_enough = true;
    for (int i = leave_index; i < arrive_index; ++i) {
        if (seats.seats[i] < n) {
            seats_are_enough = false;
            break;
        }
    }

    if (seats_are_enough) {
        order.status = 0;
        for (int i = leave_index; i < arrive_index; ++i) {
            seats.seats[i] -= n;
        }
        train_system.seats_day_file.WritePage(seats, order.seats_day); // 修改回文件
    }
    else if (!can_wait) {
        return -1;
    }
    else {
        sjtu::vector<WaitingOrder> waiting_orders = wait_list.find({{train.train_id, d}, 0}, WaitListCmp);
        order.status = waiting_orders.size() + 1;
        WaitingOrder waiting_order(leave_index, arrive_index, n, u, no);
        wait_list.insert({{train.train_id, d}, order.status}, waiting_order);
    }
    order_map.insert({u, no}, order);

    ++res.second.order_num;
    user_system.user_map.FindModify(u, true, res.second);

    if (order.status == 0) return order.price * order.num;
    return 0;
}

std::pair<bool, sjtu::vector<Order>> TicketSystem::QueryOrder(const char *u) {
    int order_num = user_system.CheckUserGetOrder(u).first;
    if (order_num == -1) return {false, {}};
    return {true, order_map.find({u, 0}, OrderMapCmp)};
}

bool TicketSystem::RefundTicket(const char *u, int n) {
    int order_num = user_system.CheckUserGetOrder(u).first;
    if (order_num == -1) return false;
    int no = order_num - n + 1;
    if (no <= 0) return false;

    Order order = order_map.FindModify({u, no}, false).second;
    if (order.status == -1) return false; // refunded: 什么都不做
    if (order.status > 0) { // pending 在waitinglist中删除，修改order状态
        wait_list.remove({{order.train_id, order.date}, order.status});
        order.status = -1; // refunded
        order_map.FindModify({u, no}, true, order);
        return true;
    }

    // order.status == 1(success)
    SeatsDay seats = train_system.seats_day_file.ReadPage(order.seats_day);
    for (int i = order.leave_index; i < order.arrive_index; ++i) {
        seats.seats[i] += order.num; // 退回座位
    }

    order.status = -1;
    order_map.FindModify({u, no}, true, order);

    // 按顺序尝试补票
    sjtu::vector<WaitingOrder> waiting_orders = wait_list.find({{order.train_id, order.date}, 0}, WaitListCmp);
    for (int i = 0; i < waiting_orders.size(); ++i) {
        CheckOrder(waiting_orders[i], seats);
    }

    train_system.seats_day_file.WritePage(seats, order.seats_day);
    return true;
}

void TicketSystem::AcceptMsg(const std::string &src) {
    sjtu::vector<std::string> res = Command::GetTokens(src, ' ');
    // 第一个是time_stamp
    std::cout << res[0] << ' ';
    // 第二个是cmd
    std::string cmd = res[1];
    int i = 2;

    if (cmd == "add_user") {
        char c[UserNameMAXLEN + 1] = {0};
        char u[UserNameMAXLEN + 1] = {0};
        char p[PassWordMAXLEN + 1] = {0};
        char n[NameMAXLEN + 1] = {0};
        char m[MailAddrMAXLEN + 1] = {0};
        int g;

        while (i < res.size()) {
            if (res[i] == "-c") {
                Command::StringToChar(c, res[++i]);
            }
            else if (res[i] == "-u") {
                Command::StringToChar(u, res[++i]);
            }
            else if (res[i] == "-p"){
                Command::StringToChar(p, res[++i]);
            }
            else if (res[i] == "-n") {
                Command::StringToChar(n, res[++i]);
            }
            else if (res[i] == "-m") {
                Command::StringToChar(m, res[++i]);
            }
            else if (res[i] == "-g") {
                g = Command::StringToInt(res[++i]);
            }
            ++i;
        }

        if (user_system.AddUser(c, u, p, n, m, g)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "login") {
        char u[UserNameMAXLEN + 1] = {0};
        char p[PassWordMAXLEN + 1] = {0};

        while (i < res.size()) {
            if (res[i] == "-u") {
                Command::StringToChar(u, res[++i]);
            }
            else if (res[i] == "-p"){
                Command::StringToChar(p, res[++i]);
            }
            ++i;
        }

        if (user_system.Login(u, p)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "logout") {
        char u[UserNameMAXLEN + 1] = {0};
        Command::StringToChar(u, res[++i]);

        if (user_system.Logout(u)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "query_profile") {
        char c[UserNameMAXLEN + 1] = {0};
        char u[UserNameMAXLEN + 1] = {0};

        while (i < res.size()) {
            if (res[i] == "-u") {
                Command::StringToChar(u, res[++i]);
            }
            else if (res[i] == "-c"){
                Command::StringToChar(c, res[++i]);
            }
            ++i;
        }

        std::pair<bool, UserInfo> user = user_system.QueryProfile(c, u);
        if (!user.first) std::cout << "-1\n";
        else std::cout << user.second;
    }
    else if (cmd == "modify_profile") {
        char c[UserNameMAXLEN + 1] = {0};
        char u[UserNameMAXLEN + 1] = {0};
        char p[PassWordMAXLEN + 1] = {0};
        char n[NameMAXLEN + 1] = {0};
        char m[MailAddrMAXLEN + 1] = {0};
        int g = -1;
        char *_p = nullptr, *_n = nullptr, *_m = nullptr;

        while (i < res.size()) {
            if (res[i] == "-c") {
                Command::StringToChar(c, res[++i]);
            }
            else if (res[i] == "-u") {
                Command::StringToChar(u, res[++i]);
            }
            else if (res[i] == "-p"){
                Command::StringToChar(p, res[++i]);
                _p = p;
            }
            else if (res[i] == "-n") {
                Command::StringToChar(n, res[++i]);
                _n = n;
            }
            else if (res[i] == "-m") {
                Command::StringToChar(m, res[++i]);
                _m = m;
            }
            else if (res[i] == "-g") {
                g = Command::StringToInt(res[++i]);
            }
            ++i;
        }

        std::pair<bool, UserInfo> user = user_system.ModifyProfile(c, u, _p, _n, _m, g);
        if (!user.first) std::cout << "-1\n";
        else std::cout << user.second;
    }
    else if (cmd == "add_train") {
        char id[TrainIDMAXLEN + 1] = {0};
        char s[StationNumMAX + 1][StaionMAXLEN + 1] = {0};
        int p[StationNumMAX + 1] = {0};
        int n = 0, m = 0, d1 = 0, d2 = 0, x = 0;
        int t[StationNumMAX + 1] = {0};
        int o[StationNumMAX + 1] = {0};
        char y;

        while (i < res.size()) {
            if (res[i] == "-i") {
                Command::StringToChar(id, res[++i]);
            }
            else if (res[i] == "-n") {
                n = Command::StringToInt(res[++i]);
            }
            else if (res[i] == "-m"){
                m = Command::StringToInt(res[++i]);
            }
            else if (res[i] == "-s") {
                sjtu::vector<std::string> stations = Command::GetTokens(res[++i], '|');
                for (int cnt = 0; cnt < stations.size(); ++cnt) {
                    Command::StringToChar(s[cnt + 1], stations[cnt]);
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
        char id[TrainIDMAXLEN + 1] = {0};
        Command::StringToChar(id, res[++i]);
        if (train_system.DeleteTrain(id)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "release_train") {
        char id[TrainIDMAXLEN + 1] = {0};
        Command::StringToChar(id, res[++i]);
        if (train_system.ReleaseTrain(id)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "query_train") {
        char id[TrainIDMAXLEN + 1] = {0};
        int d = 0;
        while (i < res.size())  {
            if (res[i] == "-i") {
                Command::StringToChar(id, res[++i]);
            }
            else if (res[i] == "-d") {
                d = Command::DateToInt(res[++i]);
            }
            ++i;
        }

        if (d == -1) {
            std::cout << "-1\n";
        }
        else {
            std::pair<std::pair<int, TrainInfo>, SeatsDay> tmp = train_system.QueryTrain(id, d);
            if (tmp.first.first == -1) std::cout << "-1\n";
            else {
                std::cout << tmp.first.second.PrintTrain(tmp.second, d);
            }
        }
    }
    else if (cmd == "query_ticket") {
        char s[StaionMAXLEN + 1] = {0};
        char t[StaionMAXLEN + 1] = {0};
        int d;
        bool p = true;

        while (i < res.size()) {
            if (res[i] == "-s") {
                Command::StringToChar(s, res[++i]);
            }
            else if (res[i] == "-t") {
                Command::StringToChar(t, res[++i]);
            }
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
            sjtu::vector<Ticket> tmp = train_system.QueryTicket(d, s, t, p);
            std::cout << tmp.size() << '\n';
            for (int cnt = 0; cnt < tmp.size(); ++cnt) {
                std::cout << tmp[cnt] << '\n';
            }
        }
    }
    else if (cmd == "query_transfer") {
        char s[StaionMAXLEN + 1] = {0};
        char t[StaionMAXLEN + 1] = {0};
        int d;
        bool p = true;

        while (i < res.size()) {
            if (res[i] == "-s") {
                Command::StringToChar(s, res[++i]);
            }
            else if (res[i] == "-t") {
                Command::StringToChar(t, res[++i]);
            }
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
            std::pair<bool, std::pair<Ticket, Ticket>> tmp = train_system.QueryTransfer(d, s, t, p);
            if (!tmp.first) std::cout << "0\n";
            else std::cout << tmp.second.first << '\n' << tmp.second.second << '\n';
        }
    }
    else if (cmd == "buy_ticket") {
        char u[UserNameMAXLEN + 1] = {0};
        char id[TrainIDMAXLEN + 1] = {0};
        int d, n;
        char f[StaionMAXLEN + 1] = {0};
        char t[StaionMAXLEN + 1] = {0};
        bool q = false;

        while (i < res.size()) {
            if (res[i] == "-u") {
                Command::StringToChar(u, res[++i]);
            }
            else if (res[i] == "-i") {
                Command::StringToChar(id, res[++i]);
            }
            else if (res[i] == "-d") {
                d = Command::DateToInt(res[++i]);
            }
            else if (res[i] == "-n") {
                n = Command::StringToInt(res[++i]);
            }
            else if (res[i] == "-f") {
                Command::StringToChar(f, res[++i]);
            }
            else if (res[i] == "-t") {
                Command::StringToChar(t, res[++i]);
            }
            else if (res[i] == "-q") {
                if (res[++i] == "true") q = true;
            }
            ++i;
        }

        if (d == -1) std::cout << "-1\n";
        else {
            int tmp = BuyTicket(u, id, d, f, t, n, q);
            if (tmp != 0) std::cout << tmp << '\n';
            else std::cout << "queue\n";
        }
    }
    else if (cmd == "query_order") {
        char u[UserNameMAXLEN + 1] = {0};
        Command::StringToChar(u, res[++i]);

        std::pair<bool, sjtu::vector<Order>> tmp = QueryOrder(u);
        if (!tmp.first) std::cout << "-1\n";
        else {
            std::cout << tmp.second.size() << '\n';
            for (int cnt = tmp.second.size() - 1 ; cnt >= 0; --cnt) {
                std::cout << tmp.second[cnt] << '\n';
            }
        }
    }
    else if (cmd == "refund_ticket") {
        char u[UserNameMAXLEN + 1] = {0};
        int n = 1;

        while (i < res.size()) {
            if (res[i] == "-u") {
                Command::StringToChar(u, res[++i]);
            }
            else if (res[i] == "-n") {
                n = Command::StringToInt(res[++i]);
            }
            ++i;
        }

        if (RefundTicket(u, n)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
}

TicketSystem::TicketSystem(const std::string &order_map, const std::string &train_system, const std::string &user_system) :
order_map(order_map + "1", order_map + "2", order_map + "3"), wait_list(order_map + "4", order_map + "5", order_map + "6"),
train_system(train_system), user_system(user_system) {}

void TicketSystem::Exit() {
    user_system.user_map.traverse(UserSystem::Operation);
}