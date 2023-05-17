
#include "TicketSystem.h"
#include <filesystem>

int main() {
    std::filesystem::path pathh{"files"};
    std::filesystem::create_directory(pathh);
    auto *ticket_system = new TicketSystem("files/order.file", "files/train.file", "files/user.file");
//    TicketSystem ticket_system("files/order.file", "files/train.file", "files/user.file");
    std::string cmd;
    while (std::getline(std::cin, cmd)) {
        std::string time_stamp;
        int i = 0;
        while (cmd[i] != ']') {
            time_stamp += cmd[i];
            ++i;
        }
        time_stamp += cmd[i];
        i += 2;
        std::string instruction = cmd.substr(i, 5);
        if (instruction == "clean") {
            delete ticket_system;
            std::filesystem::remove_all(pathh);
            std::filesystem::create_directory(pathh);
            ticket_system = new TicketSystem("files/order.file", "files/train.file", "files/user.file");
            std::cout << time_stamp << " 0\n";
        }
        else if (instruction == "exit " || instruction == "exit") {
            ticket_system->Exit();
            std::cout << time_stamp << " bye\n";
            break;
        }
        else {
            ticket_system->AcceptMsg(cmd);
        }
    }
    delete ticket_system;
    return 0;
}
