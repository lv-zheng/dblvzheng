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
	if (argc < 2 || argc > 3 || (argc == 3 && argv[1] != std::string("-i"))) {
		std::cerr << "Usage: " << argv[0] << " [-i] <db>" << std::endl;
		return 1;
	}

	bool ia = false;
	std::string dbname;
	if (argc == 3) {
		ia = true;
		dbname = argv[2];
	} else {
		dbname = argv[1];
	}

	auto idxfile = db::make_bio(dbname + ".index");
	auto stat = idxfile->stat();
	assert(stat.good); 

	auto datafile = db::make_bio(dbname + ".data");
	auto stat2 = datafile->stat();
	assert(stat2.good); 

	auto idx = db::create_index(idxfile);
	auto dbo = db::create_database(std::move(idx), datafile);

	std::string line;
	std::string op;
	while (1) {
		if (ia) {
			std::cerr << "dblz> ";
		}
		std::getline(std::cin, line);
		if (!std::cin)
			break;
		if (line.empty())
			continue;
		auto spos = line.find(' ');
		if (spos == std::string::npos || spos == 0)
			goto invalid;

		op = line.substr(0, spos);
		if (op == "insert" || op == "update") {
			auto s2pos = line.find(' ', spos + 1);
			if (s2pos == std::string::npos || s2pos == spos + 1 || s2pos + 1 == line.size())
				goto invalid;
			auto key = line.substr(spos + 1, s2pos - spos - 1);
			auto value = line.substr(s2pos + 1);
			if (op == "update") {
				bool res = dbo->remove(key);
				if (!res) {
					std::cout << "Error: entry not found." << std::endl;
					continue;
				}
			}
			try {
				dbo->insert(key, value);
			} catch (db::error::duplicate_entry&) {
				std::cout << "Error: duplicate entry." << std::endl;
			}
			//std::cout << key.size() << ' ' << key << std::endl;
			//std::cout << value.size() << ' ' << value << std::endl;
		} else if (op == "query" || op == "delete") {
			auto key = line.substr(spos + 1);
			if (key.size() == 0)
				goto invalid;
			if (op == "query") {
				try {
					auto value = dbo->query(key);
					std::cout << key << " ---> " << value << std::endl;
				} catch (db::error::key_not_found&) {
					std::cout << "Error: entry not found." << std::endl;
				}
			} else {
				bool res = dbo->remove(key);
				if (!res) {
					std::cout << "Error: entry not found." << std::endl;
					continue;
				}
				std::cout << key << " removed." << std::endl;
			}
			//std::cout << key.size() << ' ' << key << std::endl;
		} else {
			goto invalid;
		}
		continue;
invalid:
		if (!ia) {
			std::cout << "Syntax error:" << std::endl;
			std::cout << line << std::endl;
		}
		std::cerr << "Syntax error." << std::endl;
	}

	if (ia)
		std::cout << "Bye" << std::endl;

	return 0;
}
