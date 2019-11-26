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
#include <graphene/chain/protocol/asset.hpp>
#include <boost/rational.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace graphene { namespace chain {
      typedef boost::multiprecision::uint128_t uint128_t;
      typedef boost::multiprecision::int128_t  int128_t;

      bool operator == ( const price& a, const price& b )
      {
         if( std::tie( a.base.asset_id, a.quote.asset_id ) != std::tie( b.base.asset_id, b.quote.asset_id ) )
             return false;

         const auto amult = uint128_t( b.quote.amount.value ) * a.base.amount.value;
         const auto bmult = uint128_t( a.quote.amount.value ) * b.base.amount.value;

         return amult == bmult;
      }

      bool operator < ( const price& a, const price& b )
      {
         if( a.base.asset_id < b.base.asset_id ) return true;
         if( a.base.asset_id > b.base.asset_id ) return false;
         if( a.quote.asset_id < b.quote.asset_id ) return true;
         if( a.quote.asset_id > b.quote.asset_id ) return false;

         const auto amult = uint128_t( b.quote.amount.value ) * a.base.amount.value;
         const auto bmult = uint128_t( a.quote.amount.value ) * b.base.amount.value;

         return amult < bmult;
      }

      bool operator <= ( const price& a, const price& b )
      {
         return (a == b) || (a < b);
      }

      bool operator != ( const price& a, const price& b )
      {
         return !(a == b);
      }

      bool operator > ( const price& a, const price& b )
      {
         return !(a <= b);
      }

      bool operator >= ( const price& a, const price& b )
      {
         return !(a < b);
      }

      asset asset::multiply_and_round_up( const price& b )const
      {
         const asset& a = *this;
         if( a.asset_id == b.base.asset_id )
         {
            FC_ASSERT( b.base.amount.value > 0 );
            uint128_t result = (uint128_t(a.amount.value) * b.quote.amount.value + b.base.amount.value - 1)/b.base.amount.value;
            FC_ASSERT( result <= GRAPHENE_MAX_SHARE_SUPPLY );
            return asset( result.convert_to<int64_t>(), b.quote.asset_id );
         }
         else if( a.asset_id == b.quote.asset_id )
         {
            FC_ASSERT( b.quote.amount.value > 0 );
            uint128_t result = (uint128_t(a.amount.value) * b.base.amount.value + b.quote.amount.value - 1)/b.quote.amount.value;
            FC_ASSERT( result <= GRAPHENE_MAX_SHARE_SUPPLY );
            return asset( result.convert_to<int64_t>(), b.base.asset_id );
         }
         FC_THROW_EXCEPTION( fc::assert_exception, "invalid asset::multiply_and_round_up(price)", ("asset",a)("price",b) );
      }
      price operator * ( const price& a, const price& b )
      {
         price a_p = a;
         price b_p = b;
         if (a.base.asset_id == b.base.asset_id)
         {
            a_p = ~a;
            b_p = b;
         }
         else if (a.base.asset_id == b.quote.asset_id)
         {          
            a_p = ~a;
            b_p = ~b;  
         }
         else if( a.quote.asset_id == b.base.asset_id )
         {
            a_p = a;
            b_p = b;  
         }
         else if (a.quote.asset_id == b.quote.asset_id)
         {            
            a_p = a;
            b_p = ~b;  
         } else {
            FC_THROW_EXCEPTION( fc::assert_exception, "invalid ${price1} * ${price2}", ("price1",a)("price2",b) );
         }
         if (a_p.base.asset_id == a_p.quote.asset_id)
         {
            a_p.base.amount = 10000;
            a_p.quote.amount = 10000;
         }
         if (b_p.base.asset_id == b_p.quote.asset_id) 
         {
            b_p.base.amount = 10000;            
            b_p.quote.amount = 10000;
         }
         price get;
         get.base = a_p.base;
         get.quote = b_p.quote;
         uint128_t base = (uint128_t(get.base.amount.value) * b_p.base.amount.value);
         uint128_t quote = (uint128_t(get.quote.amount.value) * a_p.quote.amount.value);
         while(base > 1000000000000) {
            base  = base / 100;
            quote = quote / 100;
         }         
         get.base.amount.value = base.convert_to<int64_t>();
         get.quote.amount.value = quote.convert_to<int64_t>();
         return get;        
      }
      asset operator * ( const asset& a, const price& b )
      {
         if( a.asset_id == b.base.asset_id )
         {
            FC_ASSERT( b.base.amount.value > 0,"${id1}",("id1",b.base));
            uint128_t result = (uint128_t(a.amount.value) * b.quote.amount.value)/b.base.amount.value;
            FC_ASSERT( result <= GRAPHENE_MAX_BITCORE_SUPPLY );
            return asset( result.convert_to<int64_t>(), b.quote.asset_id );
         }
         else if( a.asset_id == b.quote.asset_id )
         {
            FC_ASSERT( b.quote.amount.value > 0 ,"${id1}",("id1",b.quote));
            uint128_t result = (uint128_t(a.amount.value) * b.base.amount.value)/b.quote.amount.value;
            FC_ASSERT( result <= GRAPHENE_MAX_BITCORE_SUPPLY );
            return asset( result.convert_to<int64_t>(), b.base.asset_id );
         }
         FC_THROW_EXCEPTION( fc::assert_exception, "invalid asset * price", ("asset",a)("price",b) );
      }

      price operator / ( const asset& base, const asset& quote )
      { try {
         FC_ASSERT( base.asset_id != quote.asset_id );
         return price{base,quote};
      } FC_CAPTURE_AND_RETHROW( (base)(quote) ) }     

      price price::max( asset_id_type base, asset_id_type quote ) { return asset( share_type(GRAPHENE_MAX_SHARE_SUPPLY), base ) / asset( share_type(1), quote); }
      price price::min( asset_id_type base, asset_id_type quote ) { return asset( 1, base ) / asset( GRAPHENE_MAX_SHARE_SUPPLY, quote); }

      price operator *  ( const price& p, const ratio_type& r )
      { try {
         p.validate();

         FC_ASSERT( r.numerator() > 0 && r.denominator() > 0 );

         if( r.numerator() == r.denominator() ) return p;

         boost::rational<int128_t> p128( p.base.amount.value, p.quote.amount.value );
         boost::rational<int128_t> r128( r.numerator(), r.denominator() );
         auto cp = p128 * r128;
         auto ocp = cp;

         bool shrinked = false;
         static const int128_t max( GRAPHENE_MAX_SHARE_SUPPLY );
         while( cp.numerator() > max || cp.denominator() > max )
         {
            if( cp.numerator() == 1 )
            {
               cp = boost::rational<int128_t>( 1, max );
               break;
            }
            else if( cp.denominator() == 1 )
            {
               cp = boost::rational<int128_t>( max, 1 );
               break;
            }
            else
            {
               cp = boost::rational<int128_t>( cp.numerator() >> 1, cp.denominator() >> 1 );
               shrinked = true;
            }
         }
         if( shrinked ) // maybe not accurate enough due to rounding, do additional checks here
         {
            int128_t num = ocp.numerator();
            int128_t den = ocp.denominator();
            if( num > den )
            {
               num /= den;
               if( num > max )
                  num = max;
               den = 1;
            }
            else
            {
               den /= num;
               if( den > max )
                  den = max;
               num = 1;
            }
            boost::rational<int128_t> ncp( num, den );
            if( num == max || den == max ) // it's on the edge, we know it's accurate enough
               cp = ncp;
            else
            {
               // from the accurate ocp, now we have ncp and cp. use the one which is closer to ocp.
               // TODO improve performance
               auto diff1 = abs( ncp - ocp );
               auto diff2 = abs( cp - ocp );
               if( diff1 < diff2 ) cp = ncp;
            }
         }

         price np = asset( cp.numerator().convert_to<int64_t>(), p.base.asset_id )
                  / asset( cp.denominator().convert_to<int64_t>(), p.quote.asset_id );

         if( ( r.numerator() > r.denominator() && np < p )
               || ( r.numerator() < r.denominator() && np > p ) )
            // even with an accurate result, if p is out of valid range, return it
            np = p;

         np.validate();
         return np;
      } FC_CAPTURE_AND_RETHROW( (p)(r.numerator())(r.denominator()) ) }

      price operator /  ( const price& p, const ratio_type& r )
      { try {
         return p * ratio_type( r.denominator(), r.numerator() );
      } FC_CAPTURE_AND_RETHROW( (p)(r.numerator())(r.denominator()) ) }

      /**
       *  The black swan price is defined as debt/collateral, we want to perform a margin call
       *  before debt == collateral.   Given a debt/collateral ratio of 1 USD / CORE and
       *  a maintenance collateral requirement of 2x we can define the call price to be
       *  2 USD / CORE.
       *
       *  This method divides the collateral by the maintenance collateral ratio to derive
       *  a call price for the given black swan ratio.
       *
       *  There exists some cases where the debt and collateral values are so small that
       *  dividing by the collateral ratio will result in a 0 price or really poor
       *  rounding errors.   No matter what the collateral part of the price ratio can
       *  never go to 0 and the debt can never go more than GRAPHENE_MAX_SHARE_SUPPLY
       *
       *  CR * DEBT/COLLAT or DEBT/(COLLAT/CR)
       */
      price price::call_price( const asset& debt, const asset& collateral, uint16_t collateral_ratio)
      { try {
         // TODO replace the calculation with new operator*() and/or operator/(), could be a hardfork change due to edge cases
         //wdump((debt)(collateral)(collateral_ratio));
         boost::rational<int128_t> swan(debt.amount.value,collateral.amount.value);
         boost::rational<int128_t> ratio( collateral_ratio, GRAPHENE_COLLATERAL_RATIO_DENOM );
         auto cp = swan * ratio;

         while( cp.numerator() > GRAPHENE_MAX_SHARE_SUPPLY || cp.denominator() > GRAPHENE_MAX_SHARE_SUPPLY )
            cp = boost::rational<int128_t>( (cp.numerator() >> 1)+1, (cp.denominator() >> 1)+1 );

         return ~(asset( cp.numerator().convert_to<int64_t>(), debt.asset_id ) / asset( cp.denominator().convert_to<int64_t>(), collateral.asset_id ));
      } FC_CAPTURE_AND_RETHROW( (debt)(collateral)(collateral_ratio) ) }

      bool price::is_null() const { return *this == price(); }     
      bool price::is_zero() const { return base.amount == 0 || quote.amount ==0; }  
      bool price::is_core() const
      {
        return (base.asset_id.get_instance() == 0 || quote.asset_id.get_instance() == 0);
      }
      bool price::is_for(const  asset_id_type asset_id ) const
      {
        return (base.asset_id == asset_id || quote.asset_id == asset_id);
      }
      bool  price::is_same(const price &other) const
      {
         return ((base.asset_id == other.base.asset_id  &&  quote.asset_id == other.quote.asset_id) || 
                 (base.asset_id == other.quote.asset_id &&  quote.asset_id == other.base.asset_id));
      }
      bool  price::is_same_asset(const price &other) const
      {
         return (base.asset_id == other.base.asset_id  &&  quote.asset_id == other.quote.asset_id);
      }
      void price::validate() const
      {
        try {
         FC_ASSERT( base.amount > share_type(0),"price base is zero" );
         FC_ASSERT( quote.amount > share_type(0) ,"price quote is zero" );
         FC_ASSERT( base.asset_id != quote.asset_id );
       
      } FC_CAPTURE_AND_RETHROW( (base)(quote) ) }

      void price_feed::validate() const
      { try {
            FC_ASSERT(!settlement_price.is_null(), "settlement_price is null");
            settlement_price.validate();

            FC_ASSERT(maximum_short_squeeze_ratio >= GRAPHENE_MIN_COLLATERAL_RATIO);
            FC_ASSERT(maximum_short_squeeze_ratio <= GRAPHENE_MAX_COLLATERAL_RATIO);

            FC_ASSERT(maintenance_collateral_ratio >= GRAPHENE_MIN_COLLATERAL_RATIO);
            FC_ASSERT(maintenance_collateral_ratio <= GRAPHENE_MAX_COLLATERAL_RATIO);

            FC_ASSERT(maximum_short_squeeze_ratio <= maintenance_collateral_ratio);

            fc::uint128 fcmax = (int64_t)0xFFFFFFFFFFFFFFFF;
            FC_ASSERT(maintenance_collateral_ratio * fc::uint128(settlement_price.base.amount.value) < fcmax);
            FC_ASSERT(maintenance_collateral_ratio * fc::uint128(settlement_price.quote.amount.value) < fcmax);

            FC_ASSERT(maintenance_collateral_ratio * fc::uint128(core_exchange_rate.base.amount.value) < fcmax);
            FC_ASSERT(maintenance_collateral_ratio * fc::uint128(core_exchange_rate.quote.amount.value) < fcmax);

            max_short_squeeze_price(); // make sure that it doesn't overflow
            max_collateral_price();
            //FC_ASSERT( maintenance_collateral_ratio >= maximum_short_squeeze_ratio );
      } FC_CAPTURE_AND_RETHROW( (*this) ) }     
      
      void   price_feed::set_pric()
      {        
          while (settlement_price.base.amount.value != 0 && settlement_price.quote.amount.value != 0 && settlement_price.base.amount.value % 10 == 0 && settlement_price.quote.amount.value % 10 == 0) 
          {
            settlement_price.base.amount /= 10;
            settlement_price.quote.amount /= 10;
          }        
          while (core_exchange_rate.base.amount.value != 0 && core_exchange_rate.quote.amount.value != 0 && core_exchange_rate.base.amount.value % 10 == 0 && core_exchange_rate.quote.amount.value % 10 == 0) 
          {
            core_exchange_rate.base.amount /= 10;
            core_exchange_rate.quote.amount /= 10;
          }
        
      }
      bool  price_feed::is_zero() const
      {
         if(settlement_price.is_null() || settlement_price.is_zero())
            return true;
         if(core_exchange_rate.is_null() || core_exchange_rate.is_zero())
            return true;
         if(settlement_price .base.asset_id ==  settlement_price .quote.asset_id )
            return true;
         if(core_exchange_rate .base.asset_id ==  core_exchange_rate .quote.asset_id )
            return true;

         return false;
      }
      bool price_feed::is_for( asset_id_type asset_id ) const
      {
         try
         {
            if( !settlement_price.is_null() )
               return settlement_price.is_for(asset_id);
            if( !core_exchange_rate.is_null() )
               return core_exchange_rate.is_for(asset_id);            
            return true;
         }
         FC_CAPTURE_AND_RETHROW( (*this) )
      }
      price price_feed::max_short_squeeze_price() const
      {        
        // TODO replace the calculation with new operator*() and/or operator/(), could be a hardfork change due to edge cases
        boost::rational<int128_t> sp(settlement_price.base.amount.value, settlement_price.quote.amount.value); //debt.amount.value,collateral.amount.value);
        boost::rational<int128_t> ratio(GRAPHENE_COLLATERAL_RATIO_DENOM, maximum_short_squeeze_ratio);
        auto cp = sp * ratio;

        while (cp.numerator() > GRAPHENE_MAX_SHARE_SUPPLY || cp.denominator() > GRAPHENE_MAX_SHARE_SUPPLY)
          cp = boost::rational<int128_t>((cp.numerator() >> 1) + (cp.numerator() & 1), (cp.denominator() >> 1) + (cp.denominator() & 1));

        return asset(cp.numerator().convert_to<int64_t>(), settlement_price.base.asset_id)/ asset(cp.denominator().convert_to<int64_t>(), settlement_price.quote.asset_id);
      }
      price price_feed::max_collateral_price() const
      {
        // TODO replace the calculation with new operator*() and/or operator/(), could be a hardfork change due to edge cases
        boost::rational<int128_t> sp(settlement_price.base.amount.value, settlement_price.quote.amount.value); //debt.amount.value,collateral.amount.value);
        boost::rational<int128_t> ratio(GRAPHENE_COLLATERAL_RATIO_DENOM, maintenance_collateral_ratio);
        auto cp = sp * ratio;

        while (cp.numerator() > GRAPHENE_MAX_SHARE_SUPPLY || cp.denominator() > GRAPHENE_MAX_SHARE_SUPPLY)
          cp = boost::rational<int128_t>((cp.numerator() >> 1) + (cp.numerator() & 1), (cp.denominator() >> 1) + (cp.denominator() & 1));

        return asset(cp.numerator().convert_to<int64_t>(), settlement_price.base.asset_id)/ asset(cp.denominator().convert_to<int64_t>(), settlement_price.quote.asset_id);
      } 
      price price_feed::user_collateral_price(uint16_t collateral_ratio ) const
      {
        // TODO replace the calculation with new operator*() and/or operator/(), could be a hardfork change due to edge cases
        boost::rational<int128_t> sp(settlement_price.base.amount.value, settlement_price.quote.amount.value); //debt.amount.value,collateral.amount.value);
        boost::rational<int128_t> ratio(GRAPHENE_COLLATERAL_RATIO_DENOM, collateral_ratio);
        auto cp = sp * ratio;

        while (cp.numerator() > GRAPHENE_MAX_SHARE_SUPPLY || cp.denominator() > GRAPHENE_MAX_SHARE_SUPPLY)
          cp = boost::rational<int128_t>((cp.numerator() >> 1) + (cp.numerator() & 1), (cp.denominator() >> 1) + (cp.denominator() & 1));

        return asset(cp.numerator().convert_to<int64_t>(), settlement_price.base.asset_id)/ asset(cp.denominator().convert_to<int64_t>(), settlement_price.quote.asset_id);
      } 
      price price_feed::maintenance_collateralization()const
      {
         if( settlement_price.is_null() )
            return price();
         return ~settlement_price * ratio_type( maintenance_collateral_ratio, GRAPHENE_COLLATERAL_RATIO_DENOM );
      }

// compile-time table of powers of 10 using template metaprogramming

template< int N >
struct p10
{
   static const int64_t v = 10 * p10<N-1>::v;
};

template<>
struct p10<0>
{
   static const int64_t v = 1;
};

const int64_t scaled_precision_lut[19] =
{
   p10<  0 >::v, p10<  1 >::v, p10<  2 >::v, p10<  3 >::v,
   p10<  4 >::v, p10<  5 >::v, p10<  6 >::v, p10<  7 >::v,
   p10<  8 >::v, p10<  9 >::v, p10< 10 >::v, p10< 11 >::v,
   p10< 12 >::v, p10< 13 >::v, p10< 14 >::v, p10< 15 >::v,
   p10< 16 >::v, p10< 17 >::v, p10< 18 >::v
};

} } // graphene::chain
