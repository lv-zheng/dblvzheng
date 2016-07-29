#include <cassert>
#include <iostream>

#include <dblvzheng/blockio.hpp>
#include <dblvzheng/index.hpp>

using namespace lvzheng;

unsigned char buff[db::consts::default_blocksize];

int main(int argc, char **argv)
{
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <test-file> <string>" << std::endl;
		return 1;
	}
	auto file = db::make_bio(argv[1]);
	auto stat = file->stat();
	assert(stat.good); 

	auto printi = [](auto a) {
		std::cout << "HASH  " << a.hash_value << std::endl;
		std::cout << "BLKNO " << a.blkno << std::endl;
		std::cout << "OFFST " << a.byte_offset << std::endl;
		std::cout << "DATA0 " << a.data0 << std::endl;
		std::cout << "DATA1 " << a.data1 << std::endl;
		std::cout << std::endl;
	};

	auto idx = db::create_index(file);
	auto q = idx->query(argv[2]);
	printi(q);

	return 0;
}
