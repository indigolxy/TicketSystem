
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
};

class UserSystem {
private:
    BPlusTree<String<UserNameMAXLEN>, UserInfo> user_map;

public:
    UserSystem() : user_map("user_file1", "user_file2", "user_file3") {}


};

#endif //TICKETSYSTEM_USERSYSTEM_H
