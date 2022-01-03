#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() { return pid_; }

float Process::CpuUtilization() {
    long activeJiffies = LinuxParser::JiffiesParser(pid_)[0];
    long idleJiffies = LinuxParser::JiffiesParser(pid_)[1];
    long totalJiffies = activeJiffies + idleJiffies;
    return (float(activeJiffies) / float(totalJiffies));
}

string Process::Command() {
    return LinuxParser::Command(pid_);
}

string Process::Ram() {
    return LinuxParser::Ram(pid_);
}

string Process::User() {
    return LinuxParser::User(pid_);
}

long int Process::UpTime() {
    return LinuxParser::UpTime(pid_);
}

bool Process::operator<(Process& a)  { 
    return (CpuUtilization() < a.CpuUtilization()) ? true : false;
}