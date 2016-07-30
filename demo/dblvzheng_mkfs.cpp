#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <dblvzheng/blockio.hpp>
#include <dblvzheng/database.hpp>
#include <dblvzheng/error.hpp>
#include <dblvzheng/index.hpp>

using namespace lvzheng;

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <db> <modulus>" << std::endl;
		return 1;
	}

	std::string dbname;
	dbname = argv[1];

	auto idxfile = db::make_bio(dbname + ".index");
	auto stat = idxfile->stat();
	assert(stat.good); 

	auto datafile = db::make_bio(dbname + ".data");
	auto stat2 = datafile->stat();
	assert(stat2.good); 

	auto modulus = std::atoll(argv[2]);

	db::super_info si;
	si.hash_modulus = modulus;
	db::mkfs_index(*idxfile, si);

	return 0;
}
