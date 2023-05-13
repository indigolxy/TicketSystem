
#include "TicketSystem.h"

int main() {
    auto *ticket_system = new TicketSystem("order.file", "train.file", "user.file");
    std::string cmd;
    while (std::getline(std::cin, cmd)) {
        int i = 1;
        while (cmd[i] != ']') ++i;
        i += 2;
        std::string instruction = cmd.substr(i);
        if (instruction == "clean") {
            delete ticket_system;
            ticket_system = new TicketSystem("order.file", "train.file", "user.file");
            std::cout << "0\n";
        }
        else if (instruction == "exit") {
            delete ticket_system;
            std::cout << "bye\n";
            return 0;
        }
        else {
            ticket_system->AcceptMsg(cmd);
        }
    }
    return 0;
}
