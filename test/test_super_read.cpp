#include <array>
#include <cassert>
#include <iostream>
#include <string>

#include <dblvzheng/blockio.hpp>
#include <dblvzheng/super.hpp>

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

	std::array<unsigned char, db::consts::default_blocksize> buf;
	auto rres = file->read(0, buf.data(), 1);
	auto si = db::super_info::from_super_block(reinterpret_cast<const db::super_block *>(buf.data()));

	return 0;
}
