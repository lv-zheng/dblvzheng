#include <cassert>
#include <cstdlib>
#include <iostream>

#include <dblvzheng/blockio.hpp>
#include <dblvzheng/database.hpp>
#include <dblvzheng/index.hpp>

using namespace lvzheng;

unsigned char buff[db::consts::default_blocksize];

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <idx> <data>" << std::endl;
		return 1;
	}
	auto idxfile = db::make_bio(argv[1]);
	auto stat = idxfile->stat();
	assert(stat.good); 

	auto datafile = db::make_bio(argv[2]);
	auto stat2 = datafile->stat();
	assert(stat2.good); 

	db::super_info si;
	si.hash_modulus = 3;
	db::mkfs_index(*idxfile, si);
	auto idx = db::create_index(idxfile);
	auto dbo = db::create_database(std::move(idx), datafile);

	dbo->insert("hello", "world");
	dbo->insert("I_love", "You");
	dbo->insert("HAHA", "haha");
	dbo->insert("HAGb", "hagb");

	std::cout << dbo->query("I_love") << std::endl;
	std::cout << dbo->query("HAHA") << std::endl;
	std::cout << dbo->query("HAGb") << std::endl;
	std::cout << dbo->query("hello") << std::endl;

	return 0;
}
