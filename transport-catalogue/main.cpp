#include "transport_catalogue.h"
#include "input_reader.h"

int main()
{
	data_base::TransportCatalogue tc;
	input::Load(std::cin, tc);
}