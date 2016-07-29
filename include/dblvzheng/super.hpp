#ifndef DBLVZHENG_SUPER_HPP
#define DBLVZHENG_SUPER_HPP

#include <cstdint>
#include <string>
#include <sys/types.h>

#include "dblvzheng/common.hpp"

namespace lvzheng {
namespace db {

struct dblvzheng_packed super_block {
	unsigned char magic[4]; // DBLZ
	unsigned char system_id[8];
	std::uint32_t version[2];

	std::uint64_t reserved_0x0014;
	std::uint32_t reserved_0x001c;

	std::uint64_t block_size;
	std::uint64_t bgroups;
	std::uint64_t bgroup_size;
	std::uint64_t reserved_blocks;

	unsigned char hash_method[16];
	std::uint64_t hash_magic;
	std::uint64_t hash_modulus;

	std::uint64_t entries;
};

static_assert(sizeof(super_block) == 104, "bad sizeof(super_block)");

struct super_info {
	std::string system_id;
	std::uint32_t version[2];

	std::uint64_t block_size;
	std::uint64_t bgroups;
	std::uint64_t bgroup_size;
	std::uint64_t reserved_blocks;

	std::string hash_method;
	std::uint64_t hash_magic;
	std::uint64_t hash_modulus;

	std::uint64_t entries;

	static super_info from_super_block(const super_block *sb);
	void to_super_block(super_block *sb);
};

} } // namespace lvzheng::db

#endif // DBLVZHENG_SUPER_HPP
