#ifndef DBLVZHENG_INDEX_HPP
#define DBLVZHENG_INDEX_HPP

#include <memory>

#include "dblvzheng/blockio.hpp"
#include "dblvzheng/super.hpp"
#include "dblvzheng/types.hpp"

namespace lvzheng {
namespace db {

struct index_entry_info {
	std::uint64_t hash_value;
	off_t blkno;
	off_t byte_offset; // byte offset in the block
	std::uint64_t data0;
	std::uint64_t data1;
};

class index {
public:
	virtual index_entry_info query(const std::string& s) = 0;
	virtual index_entry_info insert(const std::string& s, std::uint64_t data0, std::uint64_t data1) = 0;
	virtual bool remove(const std::string& s) = 0;

	virtual ~index() = default;

protected:
	index() = default;
};

std::unique_ptr<index> create_index(std::shared_ptr<blockio> bio);

void mkfs_index(blockio& bio, const super_info& si);

} } // namespace lvzheng::db

#endif // DBLVZHENG_INDEX_HPP
