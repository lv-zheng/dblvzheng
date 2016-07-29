#include "dblvzheng/hash.hpp"

#include "dblvzheng/common.hpp"
#include "dblvzheng/error.hpp"

namespace lvzheng {
namespace db {

namespace impl dblvzheng_internal {

#define IMPL_k hash_impl_k

class IMPL_k : public hash {
public:
	std::uint64_t operator () (const std::string& s) const override;
	hash_info info() const override;

	~IMPL_k() override = default;

	IMPL_k(std::uint64_t magic);

private:
	std::uint32_t _magic_init;
	std::uint32_t _magic_mul;
};

IMPL_k::IMPL_k(std::uint64_t magic)
{
	_magic_init = magic & 0xFFffFFff;
	_magic_mul = magic >> 32;
}

hash_info IMPL_k::info() const
{
	hash_info ans;
	ans.method = "djb64";
	ans.magic = _magic_init | static_cast<std::uint64_t>(_magic_mul) << 32;
	return ans;
}

std::uint64_t IMPL_k::operator () (const std::string& s) const
{
	std::uint64_t v = _magic_init;
	for (unsigned char ch : s)
		v = v * _magic_mul + ch;
	return v;
}

} // namespace impl

std::shared_ptr<hash> create_hash(const hash_info& hi)
{
	if (hi.method != "djb64")
		throw error::unknown_hash(hi.method);
	return std::make_shared<impl::IMPL_k>(hi.magic);
}

} } // namespace lvzheng::db
