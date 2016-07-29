#include <cassert>
#include <iostream>

#include <dblvzheng/blockio.hpp>
#include <dblvzheng/index.hpp>

using namespace lvzheng;

unsigned char buff[db::consts::default_blocksize];

int main(int argc, char **argv)
{
	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <test-file>" << std::endl;
		return 1;
	}
	auto file = db::make_bio(argv[1]);
	auto stat = file->stat();
	assert(stat.good); 

	auto printi = [](auto a) {
		std::cout << "HASH  " << a.hash_value << std::endl;
		std::cout << "BLKNO " << a.blkno << std::endl;
		std::cout << "OFFST " << a.byte_offset << std::endl;
		std::cout << "DENTR " << a.data_entry << std::endl;
		std::cout << std::endl;
	};

	db::super_info si;
	si.hash_modulus = 991;
	db::mkfs_index(*file, si);

	return 0;
}
