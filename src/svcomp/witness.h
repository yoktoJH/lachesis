#pragma once
#include "types.h"
#include "lachesis_types.h"
#include <vector>

struct threadData{
    LOCATION create_location;
    std::string func_name;
};

struct AccessData
{
  ACCESSREPORT variable_data;
  std::string func_name;
  //contains information on the thread creation chain from the conficting one to the main (TID 0)
  std::vector<threadData> thread_create_history;
};


void create_witness(AccessData access1,AccessData access2);