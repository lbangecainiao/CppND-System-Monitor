#include "linux_parser.h"

#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
   float Utilization();  // TODO: See src/processor.cpp

  // TODO: Declare any necessary private members
 private:
   long preActiveJiffies = 0;
   long preIdleJiffies = 0;
};

#endif