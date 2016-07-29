#include "dblvzheng/index.hpp"

#include <cstring>
#include <iostream> // debug
#include <vector>

#include <boost/endian/conversion.hpp>

#include "dblvzheng/bgroup_query.hpp"
#include "dblvzheng/consts.hpp"
#include "dblvzheng/error.hpp"
#include "dblvzheng/hash.hpp"

namespace lvzheng {
namespace db {

namespace impl dblvzheng_internal {

#define IMPL index_impl

class IMPL : public index {
public:
	index_entry_info query(const std::string& s) override;
	index_entry_info insert(const std::string& s, std::uint64_t data0, std::uint64_t data1) override;
	bool remove(const std::string& s) override;

	IMPL(std::shared_ptr<blockio> bio);
	~IMPL() override = default;

private:
	std::shared_ptr<blockio> _bio;
	std::shared_ptr<hash> _hash;
	std::unique_ptr<bgroup_query> _bgq;
	super_info _si;

	enum Operation {
		OP_query,
		OP_insert,
		OP_remove,
	};

	index_entry_info _work(const std::string& s, Operation op, std::uint64_t data0, std::uint64_t data1);
	bool _find_in_block(index_entry_info& info, const std::string& s);
	bool _try_insert(index_entry_info& info, const std::string& s);
	void _remove_entry(index_entry_info& info, size_t keylen);
};

namespace endian = boost::endian;

IMPL::IMPL(std::shared_ptr<blockio> bio):
	_bio(bio)
{
	auto stat = _bio->stat();
	if (!stat.good)
		throw error::panic();
	std::vector<unsigned char> buf(consts::default_blocksize);
	_bio->read(0, buf.data(), 1);
	_si = super_info::from_super_block(reinterpret_cast<const super_block *>(buf.data()));
	_hash = create_hash({_si.hash_method, _si.hash_magic});
	//_bio->reset_blocksize(_si.block_size);

	off_t bgoff = _si.reserved_blocks + (_si.hash_modulus * 8 - 1) / _si.block_size + 1;
	_bgq = create_bgroup_query(_bio, {bgoff, _si.bgroup_size});
}

index_entry_info IMPL::query(const std::string& s)
{
	return _work(s, OP_query, 0, 0);
}

index_entry_info IMPL::insert(const std::string& s, std::uint64_t data0, std::uint64_t data1)
{
	return _work(s, OP_insert, data0, data1);
}

bool IMPL::remove(const std::string& s)
{
	index_entry_info ans = _work(s, OP_remove, 0, 0);
	return ans.data0 != static_cast<std::uint64_t>(-1);
}

index_entry_info IMPL::_work(const std::string& s, Operation op, std::uint64_t data0, std::uint64_t data1)
{
	index_entry_info ans;

	std::uint64_t hv = (*_hash)(s) % _si.hash_modulus;
	ans.hash_value = hv;

	off_t hash_blk = _si.reserved_blocks + hv * 8 / _si.block_size;
	off_t hash_boff = hv % (_si.block_size / 8);
	std::vector<unsigned char> buf(_si.block_size);
	_bio->read(hash_blk, buf.data(), 1);

	std::uint64_t *u64buf = reinterpret_cast<std::uint64_t *>(buf.data());
	std::uint64_t head = endian::little_to_native(u64buf[hash_boff]);

	std::vector<std::uint64_t> blks;

	if (head == 0) {
		//std::cout << "DNF " << hash_blk << ' ' << hash_boff << std::endl;
		if (op == OP_query || op == OP_remove) {
			goto not_found;
		}
		head = _bgq->alloc(1, 0)[0];
		std::uint64_t head_le = endian::native_to_little(head);
		_bio->write(hash_blk, hash_boff * 8, &head_le, 8);
	}

	//std::cout << "HEAD " << head << std::endl;

	blks.push_back(head);
	while (1) {
		for (auto blk : blks) {
			if (blk == bgroup_query::eolist)
				goto not_found;
			ans.blkno = blk;
			bool suc = _find_in_block(ans, s);
			if (suc) {
				if (op == OP_query)
					return ans;
				else if (op == OP_insert)
					throw error::duplicate_entry(s);
				else {
					_remove_entry(ans, s.size());
					return ans;
				}
			}
		}
		blks = _bgq->get_next(blks.back());
	}
not_found:
	if (op == OP_query || op == OP_remove) {
		ans.blkno = -1;
		ans.byte_offset = -1;
		ans.data0 = -1;
		ans.data1 = -1;
		return ans;
	}

	//std::cout << "INSERT " << ans.blkno << ' ' << ans.byte_offset << std::endl;

	ans.data0 = data0;
	ans.data1 = data1;

	bool ok = _try_insert(ans, s);
	if (ok)
		return ans;
	auto v = _bgq->alloc(1, ans.blkno);
	ans.blkno = v[0];
	ans.byte_offset = 0;
	ok = _try_insert(ans, s);
	if (!ok)
		throw error::panic();
	return ans;
}

bool IMPL::_find_in_block(index_entry_info& info, const std::string& s)
{
	off_t blk = _bgq->get_blk_offset(info.blkno);
	std::vector<unsigned char> buf_(_si.block_size);
	unsigned char *buf = buf_.data();;
	_bio->read(blk, buf, 1);
	off_t off = 0;
	while (1) {
		if (off + 16 >= _si.block_size || buf[off + 16] == 0)
			break;

		//std::cout << "CHECK " << off << std::endl;
		std::uint64_t data0 = endian::little_to_native(
			*reinterpret_cast<std::uint64_t *>(buf + off));
		std::uint64_t data1 = endian::little_to_native(
			*reinterpret_cast<std::uint64_t *>(buf + off + 8));
		std::uint8_t len = endian::little_to_native(buf[off + 16]);
		//std::cout << "LEN " << (int) len << std::endl;

		if (len == 0)
			break;

		if (data0 == static_cast<std::uint64_t>(-1) || len != s.size())
			goto mismatch;

		if (std::memcmp(s.c_str(), buf + off + 17, len) == 0) {
			info.byte_offset = off;
			info.data0 = data0;
			info.data1 = data1;

			return true;
		}
mismatch:
		off += 16 + len;
		off = ((off - 1) / 8 + 1) * 8;
	}
	info.byte_offset = off;
	return false;
}

bool IMPL::_try_insert(index_entry_info& info, const std::string& s)
{
	if (info.byte_offset + 17 + s.size() >_si.block_size)
		return false;

	size_t len = ((17 + s.size() - 1) / 8 + 1) * 8;
	std::vector<unsigned char> buf(len, 0);

	std::uint64_t data0_le = endian::native_to_little(info.data0);
	std::uint64_t data1_le = endian::native_to_little(info.data1);
	std::memcpy(buf.data(), &data0_le, 8);
	std::memcpy(buf.data() + 8, &data1_le, 8);
	std::uint8_t len_le = endian::native_to_little(s.size());
	std::memcpy(buf.data() + 16, &len_le, 1);
	std::memcpy(buf.data() + 17, s.c_str(), len_le);

	off_t blk = _bgq->get_blk_offset(info.blkno);
	_bio->write(blk, info.byte_offset, buf.data(), buf.size());

	return true;
}

void IMPL::_remove_entry(index_entry_info& info, size_t keylen)
{
	size_t len = ((17 + keylen - 1) / 8 + 1) * 8;
	std::vector<unsigned char> buf(17 + keylen, 0);
	std::uint64_t minus_one = -1;
	minus_one = endian::native_to_little(minus_one);
	std::memcpy(buf.data(), &minus_one, 8);
	std::memcpy(buf.data() + 8, &minus_one, 8);
	buf[16] = endian::native_to_little(static_cast<std::uint8_t>(keylen));
	off_t blk = _bgq->get_blk_offset(info.blkno);
	_bio->write(blk, info.byte_offset, buf.data(), buf.size());
}

} // namespace impl

std::unique_ptr<index> create_index(std::shared_ptr<blockio> bio)
{
	return std::make_unique<impl::IMPL>(bio);
}

void mkfs_index(blockio& bio, const super_info& _si)
{
	super_info si;

	si.system_id = consts::system_id;
	si.version[0] = consts::version_major;
	si.version[1] = consts::version_minor;

	si.block_size = consts::default_blocksize;
	si.bgroups = 0;
	si.bgroup_size = consts::default_bgroup_size;
	si.reserved_blocks = 2;

	si.hash_method = consts::default_hash_method;
	si.hash_magic = consts::default_hash_magic;
	si.hash_modulus = _si.hash_modulus;

	si.entries = 0;

	//bio.set_blocksize(si.blocksize);
	std::vector<unsigned char> buf(si.block_size);
	si.to_super_block(reinterpret_cast<super_block *>(buf.data()));
	bio.write(0, 0, buf.data(), si.block_size);
	
	buf.clear();
	buf.resize(si.block_size, 0);
	off_t off = si.reserved_blocks + si.hash_modulus / (si.block_size / 8);
	bio.write(off, 0, buf.data(), si.block_size);
}

} } // namespace lvzheng::db
