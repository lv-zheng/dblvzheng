#include <cassert>
#include <iostream>
#include <string>

#include <dblvzheng/hash.hpp>

using namespace lvzheng;

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <string>" << std::endl;
		return 1;
	}

	auto h = db::create_hash({"djb64", 33ULL << 32 | 5381});

	std::cout << (*h)(argv[1]) << std::endl;

	return 0;
}
