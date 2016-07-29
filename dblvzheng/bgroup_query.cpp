#include "dblvzheng/bgroup_query.hpp"

#include <iostream> // debug
#include <boost/endian/conversion.hpp>

#include "dblvzheng/common.hpp" 

namespace lvzheng {
namespace db {

namespace impl dblvzheng_internal {

#define IMPL bgroup_query_impl

class IMPL : public bgroup_query {
public:
	off_t get_bg_offset(std::uint64_t bgno) const override;
	std::uint64_t get_blk_bg(std::uint64_t blkno) const override;
	off_t get_blk_offset(std::uint64_t blkno) const override;
	std::vector<std::uint64_t> get_next(std::uint64_t blkno) override;
	std::vector<std::uint64_t> alloc(size_t n, std::uint64_t after = 0) override;

	IMPL(std::shared_ptr<blockio> bio, const bgroup_info& info);

	~IMPL() override = default;

private:
	std::shared_ptr<blockio> _bio;
	blksize_t _blocksize;
	bgroup_info _info;

	std::vector<std::uint64_t> try_alloc(std::uint64_t bgno, size_t n);
	std::vector<std::uint64_t> alloc_tail(size_t n);
	void set_llist_next(std::uint64_t which, std::uint64_t next);
};

namespace endian = boost::endian;

IMPL::IMPL(std::shared_ptr<blockio> bio, const bgroup_info& info):
	_bio(bio),
	_blocksize(bio->stat().blocksize),
	_info(info)
{
	// NOP
}

off_t IMPL::get_bg_offset(std::uint64_t bgno) const
{
	return _info.goffset + bgno * _info.bgroup_size;
}

std::uint64_t IMPL::get_blk_bg(std::uint64_t blkno) const
{
	return blkno / _info.bgroup_size;
}

off_t IMPL::get_blk_offset(std::uint64_t blkno) const
{
	return _info.goffset + blkno;
}

std::vector<uint64_t> IMPL::get_next(std::uint64_t blkno)
{
	std::uint64_t bgno = get_blk_bg(blkno);
	off_t bg_off = get_bg_offset(bgno);

	std::vector<unsigned char> buf(_blocksize);
	_bio->read(bg_off, buf.data(), 1);
	std::uint64_t *u64buf = reinterpret_cast<std::uint64_t *>(buf.data());

	std::vector<std::uint64_t> ans;
	std::uint64_t curblk = blkno;
	while (curblk != eolist && get_blk_bg(curblk) == bgno) {
		size_t entry = curblk % _info.bgroup_size;
		curblk = endian::little_to_native(u64buf[entry]);
		ans.push_back(curblk);
	}

	return ans;
}

std::vector<std::uint64_t> IMPL::alloc(size_t n, std::uint64_t after)
{
	std::vector<std::uint64_t> ans;
	if (n == 0)
		return ans;
	if (!(after == 0 || after == eolist)) {
		// allocate in the same block group
		std::uint64_t bgno = get_blk_bg(after);
		//std::cout << "BGNO " << bgno << std::endl;
		ans = try_alloc(bgno, n);

		auto s = ans.size();

		// allocate at tail
		auto tail = alloc_tail(n - ans.size());
		for (auto b : tail)
			ans.push_back(b);

		set_llist_next(ans.back(), get_next(after).front());
		if (s > 0 && s < n)
			set_llist_next(ans[s - 1], ans[s]);
		set_llist_next(after, ans.front());
	} else {
		ans = alloc_tail(n);
	}

	return ans;
}

std::vector<std::uint64_t> IMPL::try_alloc(std::uint64_t bgno, size_t n)
{
	std::vector<std::uint64_t> ans;
	if (n == 0)
		return ans;

	off_t bg_off = get_bg_offset(bgno);

	std::vector<unsigned char> buf(_blocksize);
	_bio->read(bg_off, buf.data(), 1);
	std::uint64_t *u64buf = reinterpret_cast<std::uint64_t *>(buf.data());

	for (std::uint64_t i = 1; i < _info.bgroup_size; ++i) {
		std::uint64_t next = endian::little_to_native(u64buf[i]);
		if (next == 0)
			ans.push_back(i);
		if (ans.size() == n)
			break;
	}

	if (ans.size() > 0) {
		u64buf[ans.back()] = endian::native_to_little(eolist);
		for (auto i = ans.size() - 1; i > 0; --i) {
			ans[i] += _info.bgroup_size * bgno;
			u64buf[ans[i - 1]] = endian::native_to_little(ans[i]);
		}
		ans[0] += _info.bgroup_size * bgno;
		_bio->write(bg_off, 0, buf.data(), _blocksize);
	}

	for(auto b : ans)
		//std::cout << "ALLOC " << b << std::endl;
	//std::cout << std::endl;

	return ans;
}

std::vector<std::uint64_t> IMPL::alloc_tail(size_t n)
{
	//std::cout << "TAIL " << n << std::endl;
	std::vector<std::uint64_t> ans;
	if (n == 0)
		return ans;

	auto stat = _bio->stat();
	std::uint64_t tail_bg = (stat.blockcount - _info.goffset) / _info.bgroup_size;
	if (tail_bg > 0)
		--tail_bg;

	for (; n; ++tail_bg) {
		if (ans.size()) {
			auto rslt = try_alloc(tail_bg, n);
			if (rslt.empty())
				continue;
			n -= rslt.size();
			set_llist_next(ans.back(), rslt.front());
			for (auto b : rslt)
				ans.push_back(b);
		} else {
			ans = try_alloc(tail_bg, n);
			n -= ans.size();
		}
	}

	return ans;
}

void IMPL::set_llist_next(std::uint64_t which, std::uint64_t next)
{
	//std::cout << "SETNEXT " << which << ' ' << next << std::endl;
	off_t off = get_bg_offset(get_blk_bg(which));
	std::uint64_t w = endian::native_to_little(next);
	_bio->write(off, which % _info.bgroup_size * 8, &w, 8);
}

} // namespace impl

std::unique_ptr<bgroup_query> create_bgroup_query(std::shared_ptr<blockio> bio, const bgroup_info& info)
{
	return std::make_unique<impl::IMPL>(bio, info);
}

} } // namespace lvzheng::db
