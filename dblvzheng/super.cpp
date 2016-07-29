#include "dblvzheng/super.hpp"

#include <cstring>

#include <boost/endian/conversion.hpp>

#include "dblvzheng/consts.hpp"
#include "dblvzheng/error.hpp"

namespace lvzheng {
namespace db {

namespace endian = boost::endian;

static std::string collect_string(const void *p, size_t n)
{
	const char *s = static_cast<const char *>(p);
	size_t i = 0;
	while(i < n && s[i])
		++i;
	return std::string(s, s + i);
}

super_info super_info::from_super_block(const super_block *sb)
{
	super_info si;
	
	if (std::memcmp(&sb->magic, consts::super_magic, sizeof(sb->magic)))
		throw error::bad_super("Bad DBLZ magic");

	si.system_id = collect_string(sb->system_id, sizeof(sb->system_id));
	si.version[0] = endian::little_to_native(sb->version[0]);
	si.version[1] = endian::little_to_native(sb->version[1]);

	si.block_size = endian::little_to_native(sb->block_size);
	si.bgroups = endian::little_to_native(sb->bgroups);
	si.bgroup_size = endian::little_to_native(sb->bgroup_size);
	si.reserved_blocks = endian::little_to_native(sb->reserved_blocks);

	si.hash_method = collect_string(sb->hash_method, sizeof(sb->hash_method));
	si.hash_magic = endian::little_to_native(sb->hash_magic);
	si.hash_modulus = endian::little_to_native(sb->hash_modulus);
	si.entries = endian::little_to_native(sb->entries);

	return si;
}

void super_info::to_super_block(super_block *sb)
{
	std::memcpy(&sb->magic, consts::super_magic, sizeof(sb->magic));

	std::memcpy(&sb->system_id, system_id.c_str(), system_id.size());
	sb->version[0] = endian::native_to_little(version[0]);
	sb->version[1] = endian::native_to_little(version[1]);

	sb->block_size = endian::native_to_little(block_size);
	sb->bgroups = endian::native_to_little(bgroups);
	sb->bgroup_size = endian::native_to_little(bgroup_size);
	sb->reserved_blocks = endian::native_to_little(reserved_blocks);

	std::memcpy(&sb->hash_method, hash_method.c_str(), hash_method.size());
	sb->hash_magic = endian::native_to_little(hash_magic);
	sb->hash_modulus = endian::native_to_little(hash_modulus);
	sb->entries = endian::native_to_little(entries);
}

} } // namespace lvzheng::db
