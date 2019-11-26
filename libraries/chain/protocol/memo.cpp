/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 *
 * The MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <graphene/chain/protocol/memo.hpp>
#include <fc/crypto/aes.hpp>
#include <fc/crypto/hex.hpp> 

namespace graphene { namespace chain {

//fixed_private_key: 5K3DH7oDxHe6kKjuNFUHm2qQYuYYLm9ECsmUVm2Bf4cq6Ykr8fz 
//fixed_public_key:  ZOS6FUsQ2hYRj1JSvabewWfUWTXyoDq6btmfLFjmXwby5GJgzEvT5

const private_key_type fixed_private_key = fc::ecc::private_key::generate_from_seed(fc::sha256::hash("ZOS chain 2018"));
const public_key_type  fixed_public_key = fixed_private_key.get_public_key();

void memo_data::set_message(const fc::ecc::private_key& priv, const fc::ecc::public_key& pub,
                            const string& msg, uint64_t custom_nonce)
{
   //公钥私钥不对就不需要处理。
   from = priv.get_public_key();
   to = pub;
   if(from  != public_key_type() && from  != fixed_public_key && to != public_key_type() && to != fixed_public_key && custom_nonce != unenc_nonce)
   {
      if( custom_nonce == 0 )
      {
         uint64_t entropy = fc::sha224::hash(fc::ecc::private_key::generate())._hash[0];
         entropy <<= 32;
         entropy                                                     &= 0xff00000000000000;
         nonce = (fc::time_point::now().time_since_epoch().count()   &  0x007fffffffffffff) | entropy;
      } else
         nonce = custom_nonce;

      FC_ASSERT(nonce != unenc_nonce); 
      
      auto secret = priv.get_shared_secret(pub);
      auto nonce_plus_secret = fc::sha512::hash(fc::to_string(nonce) + secret.str());
      string text = memo_message(digest_type::hash(msg)._hash[0], msg).serialize();
      message = fc::aes_encrypt( nonce_plus_secret, vector<char>(text.begin(), text.end()) );
   }
   else
   {
     this->set_message(msg);
   }
}
                            

string memo_data::get_message(const fc::ecc::private_key& priv,
                              const fc::ecc::public_key& pub)const
{
   if(nonce != unenc_nonce)
   {
      auto secret = priv.get_shared_secret(pub);
      auto nonce_plus_secret = fc::sha512::hash(fc::to_string(nonce) + secret.str());
      auto plain_text = fc::aes_decrypt( nonce_plus_secret, message );
      auto result = memo_message::deserialize(string(plain_text.begin(), plain_text.end()));
      FC_ASSERT( result.checksum == uint32_t(digest_type::hash(result.text)._hash[0]) );
      return result.text;
   }
   else
   {      
      return this->get_message();
   }
}
void memo_data::set_message(const string& msg)
{
   message.clear();
   message.assign(msg.begin(),msg.end());
   nonce = unenc_nonce; 
} 
std::string memo_data::get_message() const
{
  std::string res;
  res.insert(res.begin(), message.begin(), message.end());
  return res;
}  

string memo_message::serialize() const
{
   auto serial_checksum = string(sizeof(checksum), ' ');
   (uint32_t&)(*serial_checksum.data()) = checksum;
   return serial_checksum + text;
}

memo_message memo_message::deserialize(const string& serial)
{
   memo_message result;
   FC_ASSERT( serial.size() >= sizeof(result.checksum) );
   result.checksum = ((uint32_t&)(*serial.data()));
   result.text = serial.substr(sizeof(result.checksum));
   return result;
}

} } // graphene::chain
