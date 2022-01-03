#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>

#include "linux_parser.h"
#include "format.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  std::string filename = kProcDirectory + kMeminfoFilename;
  float memTotal = float(FileParser("MemTotal:", filename));
  float memFree = float(FileParser("MemFree:", filename));
  float memUsed = memTotal - memFree;
  return (memUsed / memTotal);
}

long LinuxParser::UpTime() {
  string uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return stol(uptime);
}

long LinuxParser::Jiffies() { return  JiffiesParser()[0] +  JiffiesParser()[1]; }

long LinuxParser::ActiveJiffies(int pid) { return JiffiesParser(pid)[0]; }

long LinuxParser::ActiveJiffies() { return JiffiesParser()[0]; }

long LinuxParser::IdleJiffies() { return JiffiesParser()[1]; }


vector<long> LinuxParser::JiffiesParser(int pid) {
  string fileName = kProcDirectory + "/" + std::to_string(pid) + kStatFilename;
  string line;
  string value;
  long utime;
  long stime;
  long cutime;
  long cstime;
  long starttime;
  long activeJiffies;
  long totalJiffies;
  vector<long> JiffiesRst;
  int iter = 0;
  std::ifstream filestream(fileName);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      switch (iter) {
        case 13:
            utime = stol(value);
            iter++;
            continue;
        case 14:
            stime = stol(value);
            iter++;
            continue;
        case 15:
            cutime = stol(value);
            iter++;
            continue;
        case 16:
            cstime = stol(value);
            iter++;
            continue;
        case 21:
            starttime = stol(value);
            iter++;
            continue;
      }
      iter++;
    }
    long sysUpTime = UpTime();
    activeJiffies = utime + stime + cutime + cstime;
    totalJiffies = sysUpTime * sysconf(_SC_CLK_TCK) - starttime;
  }
  JiffiesRst.emplace_back(activeJiffies);
  JiffiesRst.emplace_back(totalJiffies - activeJiffies);
  return JiffiesRst;
}

vector<int> LinuxParser::JiffiesParser() {
  string line;
  string key;
  string user;
  string nice;
  string system;
  string idle;
  string iowait;
  string irq;
  string softirq;
  string steal;
  string fileName = kProcDirectory + kStatFilename;
  vector<int> jiffiesRst;

  std::ifstream filestream(fileName);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        if (key == "cpu") {
          linestream >> user >> nice >> system >> idle
            >> iowait >> irq >> softirq >> steal;
          int activeJiffies = stoi(user) + stoi(nice) +
            stoi(system) + stoi(irq) + stoi(softirq) + stoi(steal);
          int idleJiffies = stoi(idle) + stoi(iowait);
          jiffiesRst.emplace_back(activeJiffies);
          jiffiesRst.emplace_back(idleJiffies);
          break;
        }
      }
    }
  }
  return jiffiesRst;
}

vector<string> LinuxParser::CpuUtilization() {
  int activeJiffies = JiffiesParser()[0];
  int idleJiffies = JiffiesParser()[1];
  int totalJiffies = activeJiffies + idleJiffies;
  float cpuUtilization = (float(activeJiffies) / float(totalJiffies));

  return {std::to_string(cpuUtilization)}; 
}

int LinuxParser::TotalProcesses() {
  std::string fileName = kProcDirectory + kStatFilename;
  return FileParser("processes", fileName);
}

int LinuxParser::RunningProcesses() {
  std::string fileName = kProcDirectory + kStatFilename;
  return FileParser("procs_running", fileName);
}

int LinuxParser::FileParser(std::string keyTarget, std::string fileName) { 
  string line;
  string key;
  string value;

  std::ifstream filestream(fileName);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == keyTarget) {
          return stoi(value);
        }
      }
    }
  }
  return 0;
}

string LinuxParser::Command(int pid) { 
  string line;
  string stream;
  string cmdline;
  string fileName = kProcDirectory + "/" + std::to_string(pid) + kCmdlineFilename;

  std::ifstream filestream(fileName);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> cmdline;
  }
  return cmdline;
}

string LinuxParser::Ram(int pid) {
  string fileName = kProcDirectory + "/" + std::to_string(pid) + kStatusFilename;
  float ramInMb = float(FileParser("VmSize:", fileName)) / 1000.0;
  string ram = std::to_string(ramInMb);
  return ram.substr(0, ram.find(".") + 3);
}

string LinuxParser::Uid(int pid) {
  string fileName = kProcDirectory + "/" + std::to_string(pid) + kStatusFilename;
  string uid = std::to_string(FileParser("Uid:", fileName));
  return uid;
}

string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line;
  string user;
  string passwd;
  string uidInPasswd;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> passwd >> uidInPasswd) {
        if (uid == uidInPasswd) {
          return user;
        }
      }
    }
  }
  return string();
}

long LinuxParser::UpTime(int pid) {
  string fileName = kProcDirectory + "/" + std::to_string(pid) + kStatFilename;
  string line;
  string value;
  int iter = 0;
  std::ifstream filestream(fileName);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
        if (iter == 21) {
          long sysUpTime = UpTime();
          return sysUpTime - (stol(value) / sysconf(_SC_CLK_TCK));
        }
        iter++;
    }
  }
  return 0;
}
