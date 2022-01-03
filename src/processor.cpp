#include <iostream>
#include "processor.h"

float Processor::Utilization() { 
    return stof(LinuxParser::CpuUtilization()[0]); 
}