#ifndef DBLVZHENG_ERROR_HPP
#define DBLVZHENG_ERROR_HPP

#include <exception>

#include "dblvzheng/blockio.hpp"
#include "dblvzheng/common.hpp"

namespace lvzheng {
namespace db {
namespace error {

struct error : std::runtime_error {
	using std::runtime_error::runtime_error;
};

struct IOError : error {
	using error::error;
	IOError(const blockio& bio):
		error("IO error on " + bio.filename())
	{ }
};

struct bad_super : error {
	using error::error;
};

struct duplicate_entry : error {
	duplicate_entry(const std::string& s):
		error("duplicate entry for " + s)
	{ }
};

struct bad_blkno : error {
	bad_blkno():
		error("bad block number")
	{ }
};

struct panic : error {
	panic():
		error("panic for unexpected error")
	{ }
};

struct unknown_hash : error {
	unknown_hash(const std::string& method):
		error("unknown hash method " + method)
	{ }
};

struct key_not_found : error {
	key_not_found(const std::string& key):
		error("key entry not found for " + key)
	{ }
};

} } } // namespace lvzheng::db::error

#endif // DBLVZHENG_ERROR_HPP
