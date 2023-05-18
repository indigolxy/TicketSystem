
#ifndef TICKETSYSTEM_USERSYSTEM_H
#define TICKETSYSTEM_USERSYSTEM_H

#include "database/BPlusTree.h"

constexpr int UserNameMAXLEN = 20;
constexpr int PassWordMAXLEN = 30;
constexpr int MailAddrMAXLEN = 30;
constexpr int NameMAXLEN = 5 * 3;

class UserInfo {
    friend class UserSystem;
    friend class TicketSystem;

private:
    char user_name[UserNameMAXLEN + 1];
    char password[PassWordMAXLEN + 1];
    char name[NameMAXLEN + 1];
    char mail_addr[MailAddrMAXLEN + 1];
    int privilege;
    bool logged_in = false;

public:
    UserInfo() = default;
    UserInfo(const char *u, const char *p, const char *n, const char *m, int pri, bool log = false) {
        strcpy(user_name, u);
        strcpy(password, p);
        strcpy(name, n);
        strcpy(mail_addr, m);
        privilege = pri;
        logged_in = log;
    }
    UserInfo(const UserInfo &other) : privilege(other.privilege), logged_in(other.logged_in) {
        strcpy(user_name, other.user_name);
        strcpy(password, other.password);
        strcpy(name, other.name);
        strcpy(mail_addr, other.mail_addr);
    }
    friend std::ostream &operator<<(std::ostream &os, const UserInfo &obj) {
        os << obj.user_name << ' ' << obj.name << ' ' << obj.mail_addr << ' '  << obj.privilege << '\n';
        return os;
    }
};

constexpr int UserMapT = ((4096 - 5) / (UserNameMAXLEN + 1 + 4) - 2) / 2;
constexpr int UserMapL = ((4096 * 2 - 8) / (UserNameMAXLEN + 1 + sizeof(UserInfo)) - 2) / 2;

class UserSystem {
    friend class TicketSystem;
private:
    BPlusTree<String<UserNameMAXLEN>, UserInfo, UserMapT, UserMapL> user_map;

    static UserInfo Operation(UserInfo obj) {
        obj.logged_in = false;
        return obj;
    }

public:
    UserSystem(const std::string &user_system) : user_map(user_system + "1", user_system + "2", user_system + "3") {}

    // * 若user不存在或未登录返回false
    std::pair<bool, UserInfo> CheckUser(const char *u) {
        std::pair<bool, UserInfo> res = user_map.FindModify(u, false);
        if (!res.first) return {false, {}}; // user doesn't exist
        if (!res.second.logged_in) return {false, {}}; // user hasn't logged in
        return {true, res.second};
    }

    bool AddUser(const char *c, const char *u, const char *p, const char *n, const char *m, int g) {
        if (user_map.empty()) {
            UserInfo target_user(u, p, n, m, 10);
            return user_map.insert(u, target_user);
        }
        std::pair<bool, UserInfo> res = user_map.FindModify(c, false);
        if (!res.first) return false; // current_user doesn't exist
        if (!res.second.logged_in) return false; // current_user hasn't logged in
        if (res.second.privilege <= g) return false; // not enough privilege
        UserInfo target_user(u, p, n, m, g);
        return user_map.insert(u, target_user);
    }

    bool Login(const char *u, const char *p) {
        std::pair<bool, UserInfo> res = user_map.FindModify(u, false);
        if (!res.first) return false; // user doesn't exist
        if (strcmp(res.second.password, p) != 0) return false; // password wrong
        if (res.second.logged_in) return false; // user has already logged in
        UserInfo new_info(res.second);
        new_info.logged_in = true;
        user_map.FindModify(u, true, new_info);
        return true;
    }

    bool Logout(const char *u) {
        std::pair<bool, UserInfo> res = user_map.FindModify(u, false);
        if (!res.first) return false; // user doesn't exist
        if (!res.second.logged_in) return false; // user hasn't logged in
        UserInfo new_info(res.second);
        new_info.logged_in = false;
        user_map.FindModify(u, true, new_info);
        return true;
    }

    std::pair<bool, UserInfo> QueryProfile(const char *c, const char *u) {
        std::pair<bool, UserInfo> res_u = user_map.FindModify(u, false);
        if (!res_u.first) return {false, {}}; // user doesn't exist

        std::pair<bool, UserInfo> res_c = user_map.FindModify(c, false);
        if (!res_c.first) return {false, {}}; // current_user doesn't exist
        if (!res_c.second.logged_in) return {false, {}}; // current_user hasn't logged in
        if (res_c.second.privilege <= res_u.second.privilege && strcmp(c, u) != 0) return {false, {}}; // not enough privilege
        return {true, res_u.second};
    }

    // * 参数省略用nullptr表示，g省略用-1表示
    std::pair<bool, UserInfo> ModifyProfile(const char *c, const char *u, const char *p, const char *n, const char *m, int g) {
        std::pair<bool, UserInfo> res_u = user_map.FindModify(u, false);
        if (!res_u.first) return {false, UserInfo()}; // user doesn't exist

        std::pair<bool, UserInfo> res_c = user_map.FindModify(c, false);
        if (!res_c.first) return {false, {}}; // current_user doesn't exist
        if (!res_c.second.logged_in) return {false, {}}; // current_user hasn't logged in
        if (res_c.second.privilege <= res_u.second.privilege && strcmp(c, u) != 0) return {false, {}}; // not enough privilege
        if (res_c.second.privilege <= g) return {false, {}}; // not enough privilege

        UserInfo new_info(res_u.second);
        if (p) strcpy(new_info.password, p);
        if (n) strcpy(new_info.name, n);
        if (m) strcpy(new_info.mail_addr, m);
        if (g != -1) new_info.privilege = g;
        user_map.FindModify(u, true, new_info);
        return {true, new_info};
    }
};

#endif //TICKETSYSTEM_USERSYSTEM_H
