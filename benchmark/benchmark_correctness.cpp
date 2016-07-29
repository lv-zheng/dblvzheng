#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>

#include <dblvzheng/blockio.hpp>
#include <dblvzheng/database.hpp>
#include <dblvzheng/error.hpp>
#include <dblvzheng/index.hpp>

using namespace lvzheng;

#define Assert(a) do { if (!(a)) std::abort(); } while (0)

int main(int argc, char **argv)
{
	if (argc != 5) {
		std::cerr << "Usage: " << argv[0] << " <idx> <data> <scale> <modulus>" << std::endl;
		return 1;
	}
	auto idxfile = db::make_bio(argv[1]);
	auto stat = idxfile->stat();
	assert(stat.good); 

	auto datafile = db::make_bio(argv[2]);
	auto stat2 = datafile->stat();
	assert(stat2.good); 

	db::super_info si;
	si.hash_modulus = std::atol(argv[4]);
	db::mkfs_index(*idxfile, si);
	auto idx = db::create_index(idxfile);
	auto dbo = db::create_database(std::move(idx), datafile);

	long scale = std::atol(argv[3]);

	std::unordered_map<std::string, std::string> duipai;
	std::default_random_engine rng{std::random_device{}()};

	for (long i = 0; i < scale; ++i) {
		std::ostringstream oss;
		oss << 'a';
		oss << i;
		std::string key = std::to_string(i);
		std::string value = "A" + key;
		dbo->insert(key, value);
		duipai[key] = value;
	}

	for (long i = 0; i < scale; ++i) {
		std::string key = std::to_string(i);
		Assert(dbo->query(key) == duipai.at(key));
	}

	long endv = 5 * scale;
	for (long i = 0; i < endv; ++i) {
		std::uniform_int_distribution<long> dist(0, scale - 1);
		long keyl = dist(rng);
		std::string key = std::to_string(keyl);
		auto it = duipai.find(key);
		if (it == duipai.end()) {
			bool flag = false;
			try {
				dbo->query(key);
			} catch (db::error::key_not_found&) {
				flag = true;
			}
			Assert(flag);
		} else {
			Assert(it->second == dbo->query(key));
		}
		if (i % 37 == 36) {
			do {
				keyl = dist(rng);
				key = std::to_string(keyl);
				it = duipai.find(key);
			} while (!duipai.count(key));
			duipai.erase(key);
			dbo->remove(key);
		}
		if (i % 11 == 10) {
			keyl = scale;
			key = std::to_string(keyl);
			duipai[key] = "A" + key;
			dbo->insert(key, "A" + key);
			++scale;
		}
		if (i % 17 == 16) {
			do {
				keyl = dist(rng);
				key = std::to_string(keyl);
				it = duipai.find(key);
			} while (!duipai.count(key));
			duipai.erase(key);
			dbo->remove(key);
			key = std::to_string(keyl);
			duipai[key] = "B" + key;
			dbo->insert(key, "B" + key);
			++scale;
		}
	}
	for (auto& p : duipai)
		dbo->remove(p.first);

	return 0;
}
