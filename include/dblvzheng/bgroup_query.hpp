#ifndef DBLVZHENG_BGROUP_QUERY_HPP
#define DBLVZHENG_BGROUP_QUERY_HPP

#include <memory>
#include <vector>

#include "dblvzheng/blockio.hpp"
#include "dblvzheng/types.hpp"

namespace lvzheng {
namespace db {

struct bgroup_info {
	off_t goffset;
	std::uint64_t bgroup_size;
};

struct bgroup_query {
public:
	virtual off_t get_bg_offset(std::uint64_t bgno) const = 0;
	virtual std::uint64_t get_blk_bg(std::uint64_t blkno) const = 0;
	virtual off_t get_blk_offset(std::uint64_t blkno) const = 0;
	virtual std::vector<std::uint64_t> get_next(std::uint64_t blkno) = 0; // may return part of the llist.
	virtual std::vector<std::uint64_t> alloc(std::size_t n, std::uint64_t after = 0) = 0;

	virtual ~bgroup_query() = default;

	static constexpr std::uint64_t eolist = static_cast<std::uint64_t>(-1);

protected:
	bgroup_query() = default;
};

std::unique_ptr<bgroup_query> create_bgroup_query(std::shared_ptr<blockio> bio, const bgroup_info& info);

} } // namespace lvzheng::db

#endif // DBLVZHENG_BGROUP_QUERY_HPP
