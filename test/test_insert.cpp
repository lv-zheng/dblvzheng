#include <cassert>
#include <cstdlib>
#include <iostream>

#include <dblvzheng/blockio.hpp>
#include <dblvzheng/index.hpp>

using namespace lvzheng;

unsigned char buff[db::consts::default_blocksize];

int main(int argc, char **argv)
{
	if (argc != 5) {
		std::cerr << "Usage: " << argv[0] << " <test-file> <string> <data0> <data1>" << std::endl;
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
	auto q = idx->insert(argv[2], std::atoi(argv[3]), std::atoi(argv[4]));
	printi(q);
	auto bak0 = q.data0;
	auto bak1 = q.data1;
	q = idx->query(argv[2]);
	printi(q);
	assert(q.data0 == bak0);
	assert(q.data1 == bak1);

	return 0;
}
