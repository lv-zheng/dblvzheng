#include "dblvzheng/database.hpp"
#include <memory>

#include "dblvzheng/blockio.hpp"
#include "dblvzheng/bgroup_query.hpp"
#include "dblvzheng/error.hpp"
#include "dblvzheng/index.hpp"

namespace lvzheng {
namespace db {

namespace impl dblvzheng_internal {

#define IMPL database_impl

class IMPL : public database {
public:
	std::string query(const std::string& key) override;
	void insert(const std::string& key, const std::string& value) override;
	bool remove(const std::string& key) override;

	IMPL(std::unique_ptr<index> idx, std::shared_ptr<blockio> datafile);
	~IMPL() override = default;

private:
	std::unique_ptr<index> _idx;
	std::shared_ptr<blockio> _data;
	std::shared_ptr<bgroup_query> _bgq;
	blksize_t _blocksize;

	std::string _get_chain(std::uint64_t head, std::uint64_t size);
	void _put_data(std::vector<std::uint64_t> storage, const std::string& value);
};

IMPL::IMPL(std::unique_ptr<index> idx, std::shared_ptr<blockio> datafile):
	_idx(std::move(idx)),
	_data(datafile)
{
	if (!_data->stat().good)
		throw error::panic();
	bgroup_info bi;
	bi.goffset = 0;
	bi.bgroup_size = consts::default_bgroup_size;
	_bgq = create_bgroup_query(_data, bi);
	_blocksize = _data->stat().blocksize;
}

std::string IMPL::query(const std::string& key)
{
	auto info = _idx->query(key);
	if (info.data0 == static_cast<std::uint64_t>(-1))
		throw error::key_not_found(key);
	return _get_chain(info.data0, info.data1);
}

void IMPL::insert(const std::string& key, const std::string& value)
{
	auto info = _idx->query(key);
	if (info.data0 != static_cast<std::uint64_t>(-1))
		throw error::duplicate_entry(key);
	auto storage = _bgq->alloc(((long) value.size() - 1) / _blocksize + 1, 0);
	_put_data(storage, value);
	auto rslt = _idx->insert(key, storage[0], value.size());
	if (rslt.data0 != storage[0] || rslt.data1 != value.size())
		throw error::panic();
}

bool IMPL::remove(const std::string& key)
{
	return _idx->remove(key);
}

std::string IMPL::_get_chain(std::uint64_t head, std::uint64_t size)
{
	std::vector<std::uint64_t> blks = {head};
	std::string ans;
	ans.resize(size);
	std::uint64_t fini = 0;
	while (1) {
		for (auto blk : blks) {
			if (blk == bgroup_query::eolist)
				goto finish;
			std::uint64_t to_read = std::min<std::uint64_t>(size - fini, _blocksize);
			std::vector<char> buf(_blocksize);
			auto off = _bgq->get_blk_offset(blk);
			_data->read(off, buf.data(), 1);
			for (std::uint64_t i = 0; i < to_read; ++i)
				ans[fini + i] = buf[i];
			fini += to_read;
		}
		blks = _bgq->get_next(blks.back());
		if (blks.empty())
			throw error::panic();
	}
finish:
	return ans;
}

void IMPL::_put_data(std::vector<std::uint64_t> storage, const std::string& value)
{
	std::uint64_t fini = 0;
	for (auto blk : storage) {
		if (fini == value.size() || blk == bgroup_query::eolist)
			break;
		std::uint64_t to_write = std::min<std::uint64_t>(value.size() - fini, _blocksize);
		auto off = _bgq->get_blk_offset(blk);
		_data->write(off, 0, value.c_str() + fini, to_write);
		fini += to_write;
	}
}

} // namespace impl

std::unique_ptr<database> create_database(std::unique_ptr<index> idx, std::shared_ptr<blockio> datafile)
{
	return std::make_unique<impl::IMPL>(std::move(idx), datafile);
}


} } // namespace lvzheng::db
