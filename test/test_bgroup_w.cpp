#include <cassert>
#include <iostream>

#include <dblvzheng/bgroup_query.hpp>
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

	db::bgroup_info bgi;
	bgi.goffset = 1;
	bgi.bgroup_size = 512;

	auto printall = [](auto a) {
		for (auto b : a)
			std::cout << b << std::endl;
		std::cout << std::endl;
	};
	auto bgq = db::create_bgroup_query(file, bgi);

	auto al = bgq->alloc(7, 0);
	printall(al);

	al = bgq->alloc(5, 2);
	printall(al);

	return 0;
}
