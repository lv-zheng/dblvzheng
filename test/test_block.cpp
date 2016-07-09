#include <cassert>
#include <iostream>

#include <dblvzheng/blockio.hpp>

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
	for (int i = 0; i < 255; i += 2) {
		buff[0] = i;
		auto wres = file->write(i, buff);
		auto stat = file->stat();
		assert(wres == stat.blocksize);
		assert(i + 1 == stat.blockcount);
	}
	return 0;
}
