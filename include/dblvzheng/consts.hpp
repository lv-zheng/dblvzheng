#ifndef DBLVZHENG_CONSTS_HPP
#define DBLVZHENG_CONSTS_HPP

#include "dblvzheng/types.hpp"

namespace lvzheng {
namespace db {
namespace consts {

constexpr blksize_t default_blocksize = 4096;
constexpr size_t default_bgroup_size = 512;
constexpr char super_magic[] = "DBLZ";
constexpr char system_id[] = "dblz-c++";
constexpr std::uint32_t version_major = 0;
constexpr std::uint32_t version_minor = 0;
constexpr char default_hash_method[] = "djb64";
constexpr std::uint64_t default_hash_magic = (33ULL << 32) | 5381;

} } } // namespace lvzheng::db::consts

#endif // DBLVZHENG_CONSTS_HPP
