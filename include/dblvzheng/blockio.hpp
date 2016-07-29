#ifndef DBLVZHENG_BLOCKIO_HPP
#define DBLVZHENG_BLOCKIO_HPP

#include <memory>

#include "dblvzheng/consts.hpp"

struct blockio_stat {
	bool good;
	off_t filesize;
	off_t blockcount;
	blksize_t blocksize;
};

namespace lvzheng {
namespace db {

// An object for block I/O for a single file. May cache.
class blockio {
public:
	// Read from file. Return the number of bytes read. -1 for error.
	virtual ssize_t read(off_t block, void *buf, size_t nblocks) = 0;

	// Write to file. Return the number of bytes written. -1 for error.
	// Writing beyond filesize will lead to expansion.
	virtual ssize_t write(off_t block, off_t offset, const void *buf, size_t nbytes) = 0;

	// Sync all write operations.
	virtual void sync() = 0;

	// Sync and close file.
	virtual void close() = 0;

	// Return stat info
	virtual blockio_stat stat() const = 0;

	virtual std::string filename() const = 0;

	// This won't be implemented or used in the near future.
#if 0
	// Reset block size. Must be power of 2. This will also change block
	// count.
	virtual ssize_t reset_blocksize(size_t bs) = 0;
#endif

	virtual ~blockio() = default;

	// Forbid cloning
	blockio(const blockio&) = delete;

protected:
	blockio() = default;
};

// Open a file for block I/O
std::shared_ptr<blockio> make_bio(const std::string& filename);

} } // namespace lvzheng::db

#endif // DBLVZHENG_BLOCKIO_HPP
