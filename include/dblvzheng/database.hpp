#ifndef DBLVZHENG_DATABASE_HPP
#define DBLVZHENG_DATABASE_HPP

#include <memory>

#include "dblvzheng/blockio.hpp"
#include "dblvzheng/index.hpp"

namespace lvzheng {
namespace db {

class database {
public:
	virtual std::string query(const std::string& key) = 0;
	virtual void insert(const std::string& key, const std::string& value) = 0;
	virtual bool remove(const std::string& key) = 0;

	virtual ~database() = default;

protected:
	database() = default;
};

std::unique_ptr<database> create_database(std::unique_ptr<index> idx, std::shared_ptr<blockio> datafile);

} } // namespace lvzheng::db

#endif // DBLVZHENG_DATABASE_HPP
