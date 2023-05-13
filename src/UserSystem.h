
#ifndef TICKETSYSTEM_USERSYSTEM_H
#define TICKETSYSTEM_USERSYSTEM_H

#include "database/FileSystem.h"
#include "database/BPlusTree.h"

constexpr int UserNameMAXLEN = 20;
constexpr int PassWordMAXLEN = 30;
constexpr int MailAddrMAXLEN = 30;
constexpr int NameMAXLEN = 5 * 3;

class UserInfo {
    friend class UserSystem;
    friend class TicketSystem;

private:
    char user_name[UserNameMAXLEN + 5];
    char password[PassWordMAXLEN + 5];
    char name[NameMAXLEN + 5];
    char mail_addr[MailAddrMAXLEN + 5];
    int privilege;
    bool logged_in = false;
    int order_num = 0; // 1-based

public:
    UserInfo() = default;
    UserInfo(const char *u, const char *p, const char *n, const char *m, int pri, bool log = false) : order_num(0) {
        strcpy(user_name, u);
        strcpy(password, p);
        strcpy(name, n);
        strcpy(mail_addr, m);
        privilege = pri;
        logged_in = log;
    }
    UserInfo(const UserInfo &other) : privilege(other.privilege), logged_in(other.logged_in), order_num(other.order_num) {
        strcpy(user_name, other.user_name);
        strcpy(password, other.password);
        strcpy(name, other.name);
        strcpy(mail_addr, other.mail_addr);
    }
    friend std::ostream &operator<<(std::ostream &os, const UserInfo &obj) {
        os << obj.user_name << ' ' << obj.name << ' ' << obj.mail_addr << ' '  << obj.privilege << '\n';
    }
};

class UserSystem {
    friend class TicketSystem;
private:
    // todo 块长
    BPlusTree<String<UserNameMAXLEN>, UserInfo> user_map;
    int user_num;

    bool CheckCurrentUser(const char *c, int least_privilege) {
        std::pair<bool, UserInfo> res = user_map.FindModify(String<UserNameMAXLEN>(c), false);
        if (!res.first) return false; // current_user doesn't exist
        if (!res.second.logged_in) return false; // current_user hasn't logged in
        if (res.second.privilege < least_privilege) return false; // not enough privilege
        return true;
    }

public:
    UserSystem(const std::string &user_system) : user_map(user_system + "1", user_system + "2", user_system + "3"), user_num(0) {}

    std::pair<int, UserInfo> CheckUserGetOrder(const char *u) {
        std::pair<bool, UserInfo> res = user_map.FindModify(u, false);
        if (!res.first) return {-1, UserInfo()}; // user doesn't exist
        if (!res.second.logged_in) return {-1, UserInfo()}; // user hasn't logged in
        return {res.second.order_num, res.second};
    }

    bool AddUser(const char *c, const char *u, const char *p, const char *n, const char *m, int g) {
        if (user_num == 0) {
            UserInfo target_user(u, p, n, m, 10);
            ++user_num;
            return user_map.insert(String<UserNameMAXLEN> (u), target_user);
        }
        if (!CheckCurrentUser(c, g + 1)) return false;
        UserInfo target_user(u, p, n, m, g);
        ++user_num;
        return user_map.insert(String<UserNameMAXLEN> (u), target_user);
    }

    bool Login(const char *u, const char *p) {
        std::pair<bool, UserInfo> res = user_map.FindModify(String<UserNameMAXLEN> (u), false);
        if (!res.first) return false; // user doesn't exist
        if (strcmp(res.second.password, p) != 0) return false; // password wrong
        if (res.second.logged_in) return false; // user has already logged in
        UserInfo new_info(res.second);
        new_info.logged_in = true;
        user_map.FindModify(String<UserNameMAXLEN> (u), true, new_info);
        return true;
    }

    bool Logout(const char *u) {
        std::pair<bool, UserInfo> res = user_map.FindModify(String<UserNameMAXLEN> (u), false);
        if (!res.first) return false; // user doesn't exist
        if (!res.second.logged_in) return false; // user hasn't logged in
        UserInfo new_info(res.second);
        new_info.logged_in = false;
        user_map.FindModify(String<UserNameMAXLEN> (u), true, new_info);
        return true;
    }

    std::pair<bool, UserInfo> QueryProfile(const char *c, const char *u) {
        std::pair<bool, UserInfo> res_u = user_map.FindModify(String<UserNameMAXLEN> (u), false);
        if (!res_u.first) return {false, UserInfo()}; // user doesn't exist
        if (!CheckCurrentUser(c, res_u.second.privilege + 1) && strcmp(c, u) != 0) return {false, UserInfo()};
        return {true, res_u.second};
    }

    // * 参数省略用nullptr表示，g省略用-1表示
    std::pair<bool, UserInfo> ModifyProfile(const char *c, const char *u, const char *p, const char *n, const char *m, int g) {
        std::pair<bool, UserInfo> res_u = user_map.FindModify(String<UserNameMAXLEN> (u), false);
        if (!res_u.first) return {false, UserInfo()}; // user doesn't exist
        if (!CheckCurrentUser(c, std::max(res_u.second.privilege, g + 1))) return {false, UserInfo()};
        UserInfo new_info(res_u.second);
        if (p) strcpy(new_info.password, p);
        if (n) strcpy(new_info.name, n);
        if (m) strcpy(new_info.mail_addr, m);
        if (g != -1) new_info.privilege = g;
        user_map.FindModify(String<UserNameMAXLEN> (u), true, new_info);
        return {true, new_info};
    }
};

#endif //TICKETSYSTEM_USERSYSTEM_H
