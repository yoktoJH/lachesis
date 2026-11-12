#pragma once
#include "types.h"
#include "lachesis_types.h"

struct threadData{
    LOCATION create_location;
    std::string func_name;
};

struct AccessData
{
  ACCESSREPORT variable_data;
  std::string func_name;
  threadData thread_data;
};


void create_witness(AccessData access1,AccessData access2);