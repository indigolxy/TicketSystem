
#ifndef TICKETSYSTEM_COMMAND_H
#define TICKETSYSTEM_COMMAND_H

class Command {
public:
    static void StringToChar(char *x, const std::string &y) {
        for (int i = 0;i < y.length();++i) {
            x[i] = y[i];
        }
    }

    static int StringToInt(const std::string &x) {
        int ans = 0;
        for (char i : x) {
            ans *= 10;
            ans += i - '0';
        }
        return ans;
    }

    static sjtu::vector<std::string> GetTokens(const std::string &src, char separator) {
        sjtu::vector<std::string> ans;
        for (int i = 0; i < src.length(); ++i) {
            std::string chunk;
            while (src[i] != separator && i < src.length()) {
                chunk += src[i];
                ++i;
            }
            ans.push_back(chunk);
        }
        return ans;
    }

    // "19:19"
    static int TimeToInt(const std::string &src) {
        Time time(src[0] * 10 + src[1], src[3] * 10 + src[4]);
        return time.TimeToInt();
    }

    // "06-01"
    static int DateToInt(const std::string &src) {
        Date date(src[0] * 10 + src[1], src[3] * 10 + src[4]);
        return date.DateToInt();
    }

    static std::string IntToDate(int d) {
        Date date(d);
        return date.DateToString();
    }

    static std::string IntToTime(int t) {
        Time time(t);
        return time.TimeToString();
    }
};

#endif //TICKETSYSTEM_COMMAND_H
