#ifndef DBLVZHENG_TYPES_HPP
#define DBLVZHENG_TYPES_HPP

#include <cstdint>
#include <sys/types.h>

namespace lvzheng {
namespace db {

using blksize_t = ::blksize_t;
using off_t = ::off_t;
using size_t = ::size_t;
using ssize_t = ::ssize_t;

} } // namespace lvzheng::db

#endif // DBLVZHENG_TYPES_HPP
