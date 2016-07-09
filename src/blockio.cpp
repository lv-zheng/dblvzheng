#include "dblvzheng/blockio.hpp"

#include <sys/fcntl.h>
#include <unistd.h>

#include "common.hpp"

namespace lvzheng {
namespace db {

namespace impl dblvzheng_internal {

class blockio_posix_impl : public blockio {
public:
	ssize_t read(off_t offset, void *buf) override;
	ssize_t write(off_t offset, const void *buf) override;
	void sync() override;
	void close() override;
	virtual blockio_stat stat() const override;

	virtual ~blockio_posix_impl() = default;

	blockio_posix_impl(const std::string& filename);

private:
	int _fd;
	off_t _blocksize;

	friend std::unique_ptr<blockio> lvzheng::db::make_bio(const std::string& filename);
};

#define IMPL blockio_posix_impl

IMPL::blockio_posix_impl(const std::string& filename)
{
	int r;

	_blocksize = consts::default_blocksize;

	_fd = ::open(filename.c_str(), O_RDWR | O_CREAT, 0644);
}

ssize_t IMPL::read(off_t offset, void *buf)
{
	if (_fd == -1)
		return -1;
	return ::pread(_fd, buf, _blocksize, offset * _blocksize);
}

ssize_t IMPL::write(off_t offset, const void *buf)
{
	if (_fd == -1)
		return -1;
	return ::pwrite(_fd, buf, _blocksize, offset * _blocksize);
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
	s.blockcount = s.filesize / s.blocksize;
	return s;
}

} // namespace impl

std::unique_ptr<blockio> make_bio(const std::string& filename)
{
	return std::make_unique<impl::IMPL>(filename);
}

} } // namespace lvzheng::db
