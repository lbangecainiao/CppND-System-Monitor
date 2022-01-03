#include <string>
#include <iostream>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) {
    string hour = std::to_string((seconds / 3600));
    string minute = std::to_string(((seconds % 3600) / 60));
    string second = std::to_string(((seconds % 3600) % 60));
    string date = hour + ":" + minute + ":" + second;
    // std::cout << "seconds format" << seconds << std::endl;

    return date;
}