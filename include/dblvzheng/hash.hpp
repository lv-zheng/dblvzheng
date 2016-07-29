#ifndef DBLVZHENG_HASH_HPP
#define DBLVZHENG_HASH_HPP

#include <memory>
#include <string>

#include "dblvzheng/types.hpp"

namespace lvzheng {
namespace db {

struct hash_info {
	std::string method;
	std::uint64_t magic;
};

class hash {
public:
	virtual std::uint64_t operator () (const std::string& s) const = 0;
	virtual hash_info info() const = 0;
	virtual ~hash() = default;

protected:
	hash() = default;
};

std::shared_ptr<hash> create_hash(const hash_info& hi);

} } // namespace lvzheng::db

#endif // DBLVZHENG_HASH_HPP
