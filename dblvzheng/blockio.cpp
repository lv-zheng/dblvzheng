#include "dblvzheng/blockio.hpp"
#include "dblvzheng/common.hpp"

#include <cstring>
#include <sys/fcntl.h>
#include <unistd.h>

namespace lvzheng {
namespace db {

namespace impl dblvzheng_internal {

class blockio_posix_impl : public blockio {
public:
	ssize_t read(off_t block, void *buf, size_t nblocks) override;
	ssize_t write(off_t block, off_t offset, const void *buf, size_t nbytes) override;
	void sync() override;
	void close() override;
	blockio_stat stat() const override;
	std::string filename() const override;

	~blockio_posix_impl() override = default;

	blockio_posix_impl(const std::string& filename);

private:
	int _fd;
	off_t _blocksize;
	std::string _filename;
};

#define IMPL blockio_posix_impl

IMPL::blockio_posix_impl(const std::string& filename):
	_filename(filename)
{
	int r;

	_blocksize = consts::default_blocksize;

	_fd = ::open(filename.c_str(), O_RDWR | O_CREAT, 0644);
}

ssize_t IMPL::read(off_t block, void *buf, size_t nblocks)
{
	if (_fd == -1)
		return -1;
	std::memset(buf, 0, nblocks * _blocksize);
	return ::pread(_fd, buf, _blocksize * nblocks, block * _blocksize);
}

ssize_t IMPL::write(off_t block, off_t offset, const void *buf, size_t nbytes)
{
	if (_fd == -1)
		return -1;
	return ::pwrite(_fd, buf, nbytes, block * _blocksize + offset);
}

void IMPL::sync()
{
	if (_fd == -1)
		return;
	::fsync(_fd);
}

void IMPL::close()
{
	if (_fd == -1)
		return;
	::close(_fd);
	_fd = -1;
}

blockio_stat IMPL::stat() const
{
	blockio_stat s{0};

	s.good = _fd != -1;
	s.blocksize = _blocksize;
	if (!s.good)
		return s;
	s.filesize = ::lseek(_fd, 0, SEEK_END);
	s.blockcount = (s.filesize - 1) / s.blocksize + 1;
	return s;
}

std::string IMPL::filename() const
{
	return _filename;
}

} // namespace impl

std::shared_ptr<blockio> make_bio(const std::string& filename)
{
	return std::make_shared<impl::IMPL>(filename);
}

} } // namespace lvzheng::db
