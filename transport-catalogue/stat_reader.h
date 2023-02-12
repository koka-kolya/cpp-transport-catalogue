#pragma once
#include "transport_catalogue.h"
#include <iostream>

namespace stat_read {

void LoadStat(std::istream& input, data_base::TransportCatalogue* tc_ptr);

}
