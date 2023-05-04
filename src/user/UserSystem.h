
#ifndef TICKETSYSTEM_USERSYSTEM_H
#define TICKETSYSTEM_USERSYSTEM_H

#include "../database/FileSystem.h"
#include "../database/BPlusTree.h"

constexpr int UserNameMAXLEN = 20;
constexpr int PassWordMAXLEN = 30;
constexpr int MailAddrMAXLEN = 30;
constexpr int NameMAXLEN = 5 * 3;

class UserInfo {
    friend class UserSystem;
private:
    char user_name[UserNameMAXLEN + 5];
    char password[PassWordMAXLEN + 5];
    char name[NameMAXLEN + 5];
    char mail_addr[MailAddrMAXLEN + 5];
    int privilege;
    bool logged_in;

public:
    UserInfo(const char *u, const char *p, const char *n, const char *m, int pri, bool log = false) {
        strcpy(user_name, u);
        strcpy(password, p);
        strcpy(name, n);
        strcpy(mail_addr, m);
        privilege = pri;
        logged_in = log;
    }
};

class UserSystem {
private:
    // todo 块长
    BPlusTree<String<UserNameMAXLEN>, UserInfo> user_map;
    int user_num;

public:
    UserSystem() : user_map("user_file1", "user_file2", "user_file3"), user_num(0) {}

    bool AddUser(const char *c, const char *u, const char *p, const char *n, const char *m, int g);

    bool Login(const char *u, const char *p);
};

#endif //TICKETSYSTEM_USERSYSTEM_H
