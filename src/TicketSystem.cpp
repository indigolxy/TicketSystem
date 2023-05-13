#include "TicketSystem.h"

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
        order.status = 1;
        for (int i = leave_index; i < arrive_index; ++i) {
            seats.seats[i] -= n;
        }
        train_system.seats_day_file.WritePage(seats, order.seats_day);
    }
    else if (!can_wait) {
        return -1;
    }
    else {
        order.status = 0;
        WaitingOrder waiting_order(leave_index, arrive_index, n, u, no);
        SeatsWaitingListDay list = train_system.seats_waiting_list_day_file.ReadPage(order.seats_waiting_list_day);
        ++list.max_index;
        list.waiting_orders[list.max_index] = waiting_order;
        train_system.seats_waiting_list_day_file.WritePage(list, order.seats_waiting_list_day);
    }
    order_map.insert({u, no}, order);

    ++res.second.order_num;
    user_system.user_map.FindModify(u, true, res.second);

    if (order.status == 1) return order.price;
    if (order.status == 0) return 0;
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
    SeatsDay seats = train_system.seats_day_file.ReadPage(order.seats_day);
    for (int i = order.leave_index; i < order.arrive_index; ++i) {
        seats.seats[i] += order.num;
    }

    order.status = -1;
    order_map.FindModify({u, no}, true, order);

    SeatsWaitingListDay list = train_system.seats_waiting_list_day_file.ReadPage(order.seats_waiting_list_day);
    for (int i = 0; i <= list.max_index; ++i) {
        CheckOrder(list.waiting_orders[i], seats, list, i);
    }

    train_system.seats_day_file.WritePage(seats, order.seats_day);
    train_system.seats_waiting_list_day_file.WritePage(list, order.seats_waiting_list_day);
}

std::pair<std::pair<std::string, std::string>, sjtu::vector<std::string>> TicketSystem::GetTokens(const std::string &s) {
    std::string time_stamp = "[";
    int i = 0;
    while (true) {
        ++i;
        time_stamp += s[i];
        if (time_stamp[i] == ']') break;
    }
    ++i;
    ++i;

    std::string cmd;
    while (s[i] != ' ') {
        cmd += s[i];
        ++i;
    }
    ++i;

    sjtu::vector<std::string> ans;
    while (i < s.size()) {
        std::string command;
        while (s[i] != ' ' && i < s.size()) {
            command += s[i];
            ++i;
        }
        ans.push_back(command);
        while (i < s.size() - 1 && s[i + 1] == ' ') { ++i; }
        ++i;
    }

    return {{time_stamp, cmd}, ans};
}

void TicketSystem::AcceptMsg(std::string s) {
    std::pair<std::pair<std::string, std::string>, sjtu::vector<std::string>> res = GetTokens(s);
    std::cout << res.first.first << ' ';
    std::string cmd = res.first.second;
    sjtu::vector<std::string> parameters = res.second;

    if (cmd == "add_user") {
        char c[UserNameMAXLEN + 5] = {0};
        char u[UserNameMAXLEN + 5] = {0};
        char p[PassWordMAXLEN + 5] = {0};
        char n[NameMAXLEN + 5] = {0};
        char m[MailAddrMAXLEN + 5] = {0};
        int g;
        int i = 0;
        while (i < parameters.size()) {
            if (parameters[i] == "-c") {
                StringToChar(c, parameters[++i]);
            }
            else if (parameters[i] == "-u") {
                StringToChar(u, parameters[++i]);
            }
            else if (parameters[i] == "-p"){
                StringToChar(p, parameters[++i]);
            }
            else if (parameters[i] == "-n") {
                StringToChar(n, parameters[++i]);
            }
            else if (parameters[i] == "-m") {
                StringToChar(m, parameters[++i]);
            }
            else if (parameters[i] == "-g") {
                g = StringToInt(parameters[++i]);
            }
            ++i;
        }
        if (user_system.AddUser(c, u, p, n, m, g)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "login") {
        char u[UserNameMAXLEN + 5] = {0};
        char p[PassWordMAXLEN + 5] = {0};
        int i = 0;
        while (i < parameters.size()) {
            if (parameters[i] == "-u") {
                StringToChar(u, parameters[++i]);
            }
            else if (parameters[i] == "-p"){
                StringToChar(p, parameters[++i]);
            }
            ++i;
        }
        if (user_system.Login(u, p)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "logout") {
        char u[UserNameMAXLEN + 5] = {0};
        StringToChar(u, parameters[1]);
        if (user_system.Logout(u)) std::cout << "0\n";
        else std::cout << "-1\n";
    }
    else if (cmd == "query_profile") {
        char c[UserNameMAXLEN + 5] = {0};
        char u[UserNameMAXLEN + 5] = {0};
        int i = 0;
        while (i < parameters.size()) {
            if (parameters[i] == "-u") {
                StringToChar(u, parameters[++i]);
            }
            else if (parameters[i] == "-c"){
                StringToChar(c, parameters[++i]);
            }
            ++i;
        }
        std::pair<bool, UserInfo> user = user_system.QueryProfile(c, u);
        if (!user.first) std::cout << "-1\n";
        else std::cout << user.second;
    }
    else if (cmd == "modify_profile") {
        char c[UserNameMAXLEN + 5] = {0};
        char u[UserNameMAXLEN + 5] = {0};
        char p[PassWordMAXLEN + 5] = {0};
        char n[NameMAXLEN + 5] = {0};
        char m[MailAddrMAXLEN + 5] = {0};
        int g = -1;
        char *_p = nullptr, *_n = nullptr, *_m = nullptr;
        int i = 0;
        while (i < parameters.size()) {
            if (parameters[i] == "-c") {
                StringToChar(c, parameters[++i]);
            }
            else if (parameters[i] == "-u") {
                StringToChar(u, parameters[++i]);
            }
            else if (parameters[i] == "-p"){
                StringToChar(p, parameters[++i]);
                _p = p;
            }
            else if (parameters[i] == "-n") {
                StringToChar(n, parameters[++i]);
                _n = n;
            }
            else if (parameters[i] == "-m") {
                StringToChar(m, parameters[++i]);
                _m = m;
            }
            else if (parameters[i] == "-g") {
                g = StringToInt(parameters[++i]);
            }
            ++i;
        }
        std::pair<bool, UserInfo> user = user_system.ModifyProfile(c, u, _p, _n, _m, g);
        if (!user.first) std::cout << "-1\n";
        else std::cout << user.second;
    }
    else if (cmd == "add_train") {

    }
    else if (cmd == "delete_train") {

    }
    else if (cmd == "release_train") {

    }
    else if (cmd == "query_train") {

    }
    else if (cmd == "query_ticket") {

    }
    else if (cmd == "query_transfer") {

    }
    else if (cmd == "buy_ticket") {

    }
    else if (cmd == "query_order") {

    }
    else if (cmd == "refund_ticket") {

    }
    else if (cmd == "clean") {

    }
    else if (cmd == "exit") {

    }
}

TicketSystem::TicketSystem(const std::string &order_map, const std::string &train_system, const std::string &user_system) :
order_map(order_map + "1", order_map + "2", order_map + "3"), train_system(train_system), user_system(user_system) {}
