#include <fc/container/flat.hpp>

#include <graphene/chain/protocol/authority.hpp>
#include <graphene/chain/protocol/operations.hpp>
#include <graphene/chain/protocol/transaction.hpp>
#include <graphene/chain/withdraw_permission_object.hpp>
#include <graphene/chain/database.hpp>
#include <graphene/chain/worker_object.hpp>
#include <graphene/business/locktoken_object.hpp>
#include <graphene/business/confidential_object.hpp>
#include <graphene/business/bitlender_option_object.hpp>
#include <graphene/business/market_object.hpp>
#include <graphene/business/bitlender_object.hpp>
#include <graphene/business/finance_object.hpp>
#include <graphene/chain/committee_member_object.hpp>
#include <graphene/chain/witness_object.hpp>
#include <graphene/chain/carrier_object.hpp>
#include <graphene/chain/gateway_object.hpp>
#include <graphene/chain/proposal_object.hpp>
#include <graphene/chain/operation_history_object.hpp>
#include <graphene/chain/vesting_balance_object.hpp>
#include <graphene/chain/transaction_object.hpp>

using namespace fc;
using namespace graphene::chain;

// TODO:  Review all of these, especially no-ops
struct get_impacted_account_visitor
{
   flat_set<account_id_type>& _impacted;
   get_impacted_account_visitor( flat_set<account_id_type>& impact ):_impacted(impact) {}
   typedef void result_type;

   void operator()( const transfer_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }
   void operator()( const transfer_vesting_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }
    void operator()( const revoke_vesting_operation& op )
   {
      _impacted.insert( op.issuer );  
   }
   
   void operator()( const asset_claim_fees_operation& op ){}
   void operator()( const asset_reserve_fees_operation& op ){}
   void operator()( const limit_order_create_operation& op ) {}
   void operator()( const limit_order_cancel_operation& op )
   {
      _impacted.insert( op.fee_paying_account );
   }
   void operator()( const call_order_update_operation& op ) {}
   void operator()( const bid_collateral_operation& op )
   {
      _impacted.insert( op.bidder );
   }
   void operator()( const fill_order_operation& op )
   {
      _impacted.insert( op.account_id );
   }
   void operator()( const execute_bid_operation& op )
   {
      _impacted.insert( op.bidder );
   }

    void operator()( const bitlender_loan_operation& op )
   {
      _impacted.insert( op.issuer );
   }
    void operator()( const bitlender_squeeze_operation& op )
   {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_remove_operation& op )
   {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_invest_operation& op ) 
   {
      _impacted.insert( op.issuer );
      _impacted.insert( op.loan_id );
   }
   void operator()( const bitlender_repay_interest_operation& op )
   {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_overdue_interest_operation& op )
      {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_recycle_interest_operation& op )
      {
      _impacted.insert( op.issuer );
   }   
   void operator()( const bitlender_repay_principal_operation& op )    {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_pre_repay_principal_operation& op )    {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_setautorepayer_operation& op )    {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_autorepayment_operation& op )    {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_overdue_repay_principal_operation& op )   {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_add_collateral_operation& op )    {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_recycle_operation op)    {
      _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_test_operation ) {} 
   void operator()( const fill_object_history_operation ) {}  

 
   void operator()( const account_create_operation& op )
   {
      _impacted.insert( op.registrar );
      _impacted.insert( op.referrer );
      add_authority_accounts( _impacted, op.owner );
      add_authority_accounts( _impacted, op.active );
   }

   void operator()( const account_update_operation& op )
   {
      _impacted.insert( op.account );
      if( op.owner )
         add_authority_accounts( _impacted, *(op.owner) );
      if( op.active )
         add_authority_accounts( _impacted, *(op.active) );
   }

   void operator()( const account_authenticate_operation& op )
   {
      _impacted.insert( op.issuer );      
   }
   void operator()( const account_config_operation& op )
   {
      _impacted.insert( op.issuer );      
   }

   

   void operator()( const account_whitelist_operation& op )
   {
      _impacted.insert( op.account_to_list );
   }

   void operator()( const account_upgrade_operation& op ) {}
   void operator()( const account_transfer_operation& op )
   {
      _impacted.insert( op.new_owner );
   }
    void operator()( const account_coupon_operation& op )
   {
      _impacted.insert( op.issuer );
   }
  

   
   void operator()( const withdraw_exchange_fee_operation& op )
   {
      _impacted.insert( op.issuer );
   }

   void operator()( const asset_property_operation& op ) 
   {
      _impacted.insert( op.issuer );
   }

   void operator()( const asset_create_operation& op ) {}
   void operator()( const asset_update_operation& op )
   {
      if( op.new_issuer )
         _impacted.insert( *(op.new_issuer) );
   }

   void operator()( const asset_update_bitasset_operation& op ) {}
   void operator()( const asset_update_gateway_operation& op ) {}
   void operator()( const asset_update_feed_producers_operation& op ) {}

   void operator()( const asset_issue_operation& op )
   {
      _impacted.insert( op.issue_to_account );
   }
   void operator()( const locktoken_option_operation& op )
   {
      _impacted.insert( op.issuer );
   }

   void operator()( const asset_reserve_operation& op ) {}
   void operator()( const asset_fund_fee_pool_operation& op ) {}
   void operator()( const asset_settle_operation& op ) {}
   void operator()( const asset_global_settle_operation& op ) {}
   void operator()( const asset_publish_feed_operation& op ) {}

   void operator()( const bitlender_option_create_operation& op )
    {
         _impacted.insert( op.issuer );
    }
    void operator()( const bitlender_option_author_operation& op )
    {
         _impacted.insert( op.issuer );
    }
   void operator()( const bitlender_option_update_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_option_fee_mode_operation& op )
   {
        _impacted.insert( op.issuer );
   }
    void operator()( const bitlender_option_stop_operation& op )
   {
        _impacted.insert( op.issuer );
   }
  void operator()( const bitlender_paramers_update_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_rate_update_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const bitlender_publish_feed_operation& op ) {}
   void operator()( const bitlender_update_feed_producers_operation& op ) {}

   void operator()( const  issue_fundraise_create_operation& op ) 
   {
      _impacted.insert( op.issuer );
   }
    void operator()( const  issue_fundraise_publish_feed_operation& op ) 
   {
      _impacted.insert( op.issuer );
   }
    void operator()( const  finance_paramers_update_operation& op ) 
   {
      _impacted.insert( op.issuer );
   }
   void operator()( const  issue_fundraise_update_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const  issue_fundraise_remove_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const buy_fundraise_create_operation& op ) 
   {
      _impacted.insert( op.issuer );
   }
  
   void operator()( const buy_fundraise_enable_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const sell_exchange_create_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const sell_exchange_update_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const sell_exchange_remove_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const buy_exchange_create_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const buy_exchange_update_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const buy_exchange_remove_operation& op )
   {
        _impacted.insert( op.issuer );
   }
   void operator()( const witness_create_operation& op )
   {
      _impacted.insert( op.witness_account );
   }
   void operator()( const witness_update_operation& op )
   {
      _impacted.insert( op.witness_account );
   }
   void operator()( const change_identity_operation& op )
   {
      _impacted.insert( op.issuer );
   }  
   void operator()( const gateway_update_operation& op )
   {
      _impacted.insert( op.gateway_account );
   }
   void operator()( const gateway_create_operation& op )
   {
      _impacted.insert( op.gateway_account );
   }
   void operator()( const carrier_update_operation& op )
   {
      _impacted.insert( op.carrier_account );
   }
   void operator()( const carrier_create_operation& op )
   {
      _impacted.insert( op.carrier_account );
   }
   void operator()( const author_create_operation& op )
   {
      _impacted.insert( op.author_account );
   }
   void operator()( const author_update_operation& op )
   {
      _impacted.insert( op.author_account );
   }


   void operator()( const proposal_create_operation& op )
   {
      vector<authority> other;
      for( const auto& proposed_op : op.proposed_ops )
         operation_get_required_authorities( proposed_op.op, _impacted, _impacted, other );
      for( auto& o : other )
         add_authority_accounts( _impacted, o );
   }

   void operator()( const proposal_update_operation& op ) {}
   void operator()( const proposal_delete_operation& op ) {}

   void operator()( const withdraw_permission_create_operation& op )
   {
      _impacted.insert( op.authorized_account );
   }

   void operator()( const withdraw_permission_update_operation& op )
   {
      _impacted.insert( op.authorized_account );
   }

   void operator()( const withdraw_permission_claim_operation& op )
   {
      _impacted.insert( op.withdraw_from_account );
   }

   void operator()( const withdraw_permission_delete_operation& op )
   {
      _impacted.insert( op.authorized_account );
   }

   void operator()( const committee_member_create_operation& op )
   {
      _impacted.insert( op.committee_member_account );
   }
   void operator()( const committee_member_update_operation& op )
   {
      _impacted.insert( op.committee_member_account );
   }
   void operator()( const committee_member_update_global_parameters_operation& op ) {}
   void operator()( const committee_member_update_zos_parameters_operation& op ) {}
  

   void operator()( const budget_member_create_operation& op )
   {
      _impacted.insert( op.budget_member_account );
   }
   void operator()( const budget_member_update_operation& op )
   {
      _impacted.insert( op.budget_member_account );
   }

    void operator()( const gateway_issue_currency_operation& op )
   {
      _impacted.insert( op.issuer );
      _impacted.insert( op.account_to );
   }
   
   void operator()( const gateway_withdraw_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to );
   }
   void operator()( const gateway_deposit_operation& op )
   {
      _impacted.insert( op.from );
      _impacted.insert( op.to ); 
   }
  
   void operator()( const vesting_balance_create_operation& op )
   {
      _impacted.insert( op.owner );
   }

   void operator()( const vesting_balance_withdraw_operation& op ) {}
   void operator()( const worker_create_operation& op ) {}
   void operator()( const custom_operation& op ) {}
   void operator()( const assert_operation& op ) {}
   void operator()( const balance_claim_operation& op ) {}

   void operator()( const locktoken_create_operation& op ) { _impacted.insert( op.issuer );}
   void operator()( const locktoken_update_operation& op ) { _impacted.insert( op.issuer );}
   void operator()( const locktoken_node_operation& op )   { _impacted.insert( op.issuer );}
   void operator()( const locktoken_remove_operation& op ) { _impacted.insert( op.issuer );}

   void operator()( const override_transfer_operation& op )
   {
      _impacted.insert( op.to );
      _impacted.insert( op.from );
      _impacted.insert( op.issuer );
   }

   void operator()( const transfer_to_blind_operation& op )
   {
      _impacted.insert( op.from );
      for( const auto& out : op.outputs )
         add_authority_accounts( _impacted, out.owner );
   }

   void operator()( const blind_transfer_operation& op )
   {
      for( const auto& in : op.inputs )
         add_authority_accounts( _impacted, in.owner );
      for( const auto& out : op.outputs )
         add_authority_accounts( _impacted, out.owner );
   }

   void operator()( const transfer_from_blind_operation& op )
   {
      _impacted.insert( op.to );
      for( const auto& in : op.inputs )
         add_authority_accounts( _impacted, in.owner );
   }

   void operator()( const asset_settle_cancel_operation& op )
   {
      _impacted.insert( op.account );
   }

   void operator()( const fba_distribute_operation& op )
   {
      _impacted.insert( op.account_id );
   }

};

static void operation_get_impacted_accounts( const operation& op, flat_set<account_id_type>& result )
{
  get_impacted_account_visitor vtor = get_impacted_account_visitor( result );
  op.visit( vtor );
}

static void transaction_get_impacted_accounts( const transaction& tx, flat_set<account_id_type>& result )
{
  for( const auto& op : tx.operations )
    operation_get_impacted_accounts( op, result );
}

static void get_relevant_accounts( const object* obj, flat_set<account_id_type>& accounts )
{
   if( obj->id.space() == protocol_ids )
   {
      switch( (object_type)obj->id.type() )
      {
        case null_object_type:
        case base_object_type:
        case OBJECT_TYPE_COUNT:
           return;
        case account_object_type:{
           accounts.insert( obj->id );
           break;
        } case asset_object_type:{
           const auto& aobj = dynamic_cast<const asset_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;
        } case bitlender_option_object_type:{
           const auto& aobj = dynamic_cast<const bitlender_option_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;
        } case issue_fundraise_object_type:{
           const auto& aobj = dynamic_cast<const issue_fundraise_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;              
        } case force_settlement_object_type:{
           const auto& aobj = dynamic_cast<const force_settlement_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->owner );
           break;
        } case committee_member_object_type:{
           const auto& aobj = dynamic_cast<const committee_member_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->committee_member_account );
           break;
        }  case budget_member_object_type:{
           const auto& aobj = dynamic_cast<const budget_member_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->budget_member_account );
           break;
        }   case witness_object_type:{
           const auto& aobj = dynamic_cast<const witness_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->witness_account );
           break;
        }  case gateway_object_type:{
           const auto& aobj = dynamic_cast<const gateway_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->gateway_account );
           break;
        }  case  carrier_object_type:{
           const auto& aobj = dynamic_cast<const carrier_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->carrier_account );
           break;
        } case  author_object_type:{
           const auto& aobj = dynamic_cast<const author_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->author_account );
           break;
        } case bitlender_order_object_type:{
           const auto& aobj = dynamic_cast<const bitlender_order_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;
        } case buy_fundraise_object_type:{
           const auto& aobj = dynamic_cast<const buy_fundraise_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;
        } case call_order_object_type:{
           const auto& aobj = dynamic_cast<const call_order_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->borrower );
           break;
        } case custom_object_type:{
          break;
        } case proposal_object_type:{
           const auto& aobj = dynamic_cast<const proposal_object*>(obj);
           assert( aobj != nullptr );
           transaction_get_impacted_accounts( aobj->proposed_transaction, accounts );
           break;
        } case operation_history_object_type:{
           const auto& aobj = dynamic_cast<const operation_history_object*>(obj);
           assert( aobj != nullptr );
           operation_get_impacted_accounts( aobj->op, accounts );
           break;
        } case withdraw_permission_object_type:{
           const auto& aobj = dynamic_cast<const withdraw_permission_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->withdraw_from_account );
           accounts.insert( aobj->authorized_account );
           break;
        } case vesting_balance_object_type:{
           const auto& aobj = dynamic_cast<const vesting_balance_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->owner );
           break;
        } case worker_object_type:{
           const auto& aobj = dynamic_cast<const worker_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->worker_account );
           break;
        } case locktoken_object_type:{
           const auto& aobj = dynamic_cast<const locktoken_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;
        } case locktoken_options_object_type:{
           const auto& aobj = dynamic_cast<const locktoken_option_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;
        } case limit_order_object_type:{
           const auto& aobj = dynamic_cast<const limit_order_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->seller );
           break;
        } case sell_exchange_object_type:{
           const auto& aobj = dynamic_cast<const sell_exchange_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;
        } case buy_exchange_object_type:{
           const auto& aobj = dynamic_cast<const buy_exchange_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;
        }  case balance_object_type:{
           /** these are free from any accounts */
           break;
        }
      }
   }
   else if( obj->id.space() == implementation_ids )
   {
      switch( (impl_object_type)obj->id.type() )
      {
             case impl_global_property_object_type:
              break;
             case impl_dynamic_global_property_object_type:
              break;
             case impl_reserved0_object_type:
              break;
             case impl_asset_dynamic_data_type:
              break;
             case impl_asset_bitasset_data_type:
              break;
            case impl_asset_exchange_feed_type:
              break;  
             case impl_account_balance_object_type:{
              const auto& aobj = dynamic_cast<const account_balance_object*>(obj);
              assert( aobj != nullptr );
              accounts.insert( aobj->owner );
              break;
           } case impl_account_statistics_object_type:{
              const auto& aobj = dynamic_cast<const account_statistics_object*>(obj);
              assert( aobj != nullptr );
              accounts.insert( aobj->owner );
              break;
           } case impl_transaction_object_type:{
              const auto& aobj = dynamic_cast<const transaction_object*>(obj);
              assert( aobj != nullptr );
              transaction_get_impacted_accounts( aobj->trx, accounts );
              break;
           } case impl_blinded_balance_object_type:{
              const auto& aobj = dynamic_cast<const blinded_balance_object*>(obj);
              assert( aobj != nullptr );
              for( const auto& a : aobj->owner.account_auths )
                accounts.insert( a.first );
              break;
           } case impl_block_summary_object_type:
              break;
             case impl_account_transaction_history_object_type:
              break;
             case impl_account_balance_history_object_type:
              break;          
             case impl_bitlender_paramers_object_type:
               break;           
             case  impl_finance_paramers_object_type:
               break;
             case  impl_carrier_dynamic_object_type:
               break;
             case  impl_gateway_dynamic_object_type:
               break;
             case  impl_locktoken_history_object_type:
               break;
             case impl_chain_property_object_type:
               break;
             case impl_witness_schedule_object_type:
              break;
             case impl_budget_record_object_type:
              break;
             case impl_special_authority_object_type:
              break;
             case impl_buyback_object_type:
              break;
             case impl_fba_accumulator_object_type:
              break;
             case impl_collateral_bid_object_type:{
              const auto& aobj = dynamic_cast<const collateral_bid_object*>(obj);
              assert( aobj != nullptr );
              accounts.insert( aobj->bidder );
              break;
           } case impl_bitlender_invest_object_type:{
           const auto& aobj = dynamic_cast<const bitlender_invest_object*>(obj);
           assert( aobj != nullptr );
           accounts.insert( aobj->issuer );
           break;
           }  
       }
   }
} // end get_relevant_accounts( const object* obj, flat_set<account_id_type>& accounts )

namespace graphene { namespace chain {

void database::notify_applied_block( const signed_block& block )
{
   GRAPHENE_TRY_NOTIFY( applied_block, block )
}

void database::notify_on_pending_transaction( const signed_transaction& tx )
{
   GRAPHENE_TRY_NOTIFY( on_pending_transaction, tx )
}

void database::notify_changed_objects()
{ try {
   if( _undo_db.enabled() ) 
   {
      const auto& head_undo = _undo_db.head();

      // New
      if( !new_objects.empty() )
      {
        vector<object_id_type> new_ids;  new_ids.reserve(head_undo.new_ids.size());
        flat_set<account_id_type> new_accounts_impacted;
        for( const auto& item : head_undo.new_ids )
        {
          new_ids.push_back(item);
          auto obj = find_object(item);
          if(obj != nullptr)
            get_relevant_accounts(obj, new_accounts_impacted);
        }

        if( new_ids.size() )
           GRAPHENE_TRY_NOTIFY( new_objects, new_ids, new_accounts_impacted)
      }

      // Changed
      if( !changed_objects.empty() )
      {
        vector<object_id_type> changed_ids;  changed_ids.reserve(head_undo.old_values.size());
        flat_set<account_id_type> changed_accounts_impacted;
        for( const auto& item : head_undo.old_values )
        {
          changed_ids.push_back(item.first);
          get_relevant_accounts(item.second.get(), changed_accounts_impacted);
        }

        if( changed_ids.size() )
           GRAPHENE_TRY_NOTIFY( changed_objects, changed_ids, changed_accounts_impacted)
      }

      // Removed
      if( !removed_objects.empty() )
      {
        vector<object_id_type> removed_ids; removed_ids.reserve( head_undo.removed.size() );
        vector<const object*> removed; removed.reserve( head_undo.removed.size() );
        flat_set<account_id_type> removed_accounts_impacted;
        for( const auto& item : head_undo.removed )
        {
          removed_ids.emplace_back( item.first );
          auto obj = item.second.get();
          removed.emplace_back( obj );
          get_relevant_accounts(obj, removed_accounts_impacted);
        }

        if( removed_ids.size() )
           GRAPHENE_TRY_NOTIFY( removed_objects, removed_ids, removed, removed_accounts_impacted)
      }
   }
} FC_CAPTURE_AND_LOG( (0) ) }

} }
