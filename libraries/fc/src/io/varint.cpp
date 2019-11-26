#include <fc/io/varint.hpp>
#include <fc/variant.hpp>

namespace fc
{
void to_variant( const signed_int& var, variant& vo, uint32_t max_depth ) { vo = var.value; }
void from_variant( const variant& var, signed_int& vo, uint32_t max_depth ) { vo.value = static_cast<int32_t>(var.as_int64()); }
void to_variant( const unsigned_int& var, variant& vo, uint32_t max_depth )  { vo = var.value; }
void from_variant( const variant& var, unsigned_int& vo, uint32_t max_depth )  { vo.value = static_cast<uint32_t>(var.as_uint64()); }
void to_variant( const unsigned_int64& var, variant& vo, uint32_t max_depth )  { vo = var.value; }
void from_variant( const variant& var, unsigned_int64& vo, uint32_t max_depth )  { vo.value = static_cast<uint64_t>(var.as_uint64()); }
}
