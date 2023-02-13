#pragma once
#include "transport_catalogue.h"
#include <iostream>

namespace stat_read {

std::ostream& ReadBase(std::istream& input, data_base::TransportCatalogue* tc_ptr, std::ostream& out);

}
