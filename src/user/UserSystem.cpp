#include "UserSystem.h"

bool UserSystem::AddUser(const char *c, const char *u, const char *p, const char *n, const char *m, int g) {
    if (user_num == 0) {
        UserInfo target_user(u, p, n, m, 10);
        ++user_num;
        return user_map.insert(String<UserNameMAXLEN> (u), target_user);
    }
    sjtu::vector<UserInfo> res = user_map.find(String<UserNameMAXLEN>(c), String_comp);
    if (res.empty()) return false; // current_user doesn't exist
    if (!res.front().logged_in) return false; // current_user hasn't logged in
    if (res.front().privilege <= g) return false; // current_user's privilege <= target_user
    UserInfo target_user(u, p, n, m, g);
    ++user_num;
    return user_map.insert(String<UserNameMAXLEN> (u), target_user);
}

bool UserSystem::Login(const char *u, const char *p) {

}