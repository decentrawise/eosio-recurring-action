#include <eosio/eosio.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>

#include "../../src/recurring_action.hpp"


// #define RECURRING_ACTION eosio::name("count")
#define RECURRING_ACTION eosio::name("tick")
#define RECURRING_DELAYSEC 1

class [[eosio::contract]] profile: public eosio::contract,
                                          recurring_action {

public:

  profile(eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds) :
    contract(receiver, code, ds),
    recurring_action(receiver, RECURRING_DELAYSEC, RECURRING_ACTION)
  {}

  [[eosio::action]]
  void update(eosio::name user, std::string nickname, std::string avatar,
              std::string website, std::string locale, std::string metadata) {
    require_auth(user);

    profiles_table profiles(_self, _self.value);
    auto iterator = profiles.find(user.value);
    if (iterator == profiles.end()) {
      // Create new
      profiles.emplace(user, [&](auto & row) {
        row.user = user;
        row.nickname = nickname;
        row.avatar = avatar;
        row.website = website;
        row.locale = locale;
        row.metadata = metadata;
      });
    } else {
      // Update existing
      profiles.modify(iterator, eosio::same_payer, [&](auto & row) {
        row.user = user;
        row.nickname = nickname;
        row.avatar = avatar;
        row.website = website;
        row.locale = locale;
        row.metadata = metadata;
      });
    }
  }

  [[eosio::action]]
  void remove(eosio::name user) {
    require_auth(user);

    profiles_table profiles(_self, _self.value);
    auto iterator = profiles.find(user.value);
    eosio::check(iterator != profiles.end(), "User doesn't have a profile");
    profiles.erase(iterator);
  }

  [[eosio::action]]
  void count() {
    profiles_table profiles(_self, _self.value);
    auto iterator = profiles.begin();
    eosio::check(iterator != profiles.end(), "No user profiles yet");
    // limit to 20 to check that an assert in the recurring action doesn't break the chain...
    eosio::check(iterator->count < 20, "Limit of 20 reached");
    profiles.modify(iterator, eosio::same_payer, [&](auto & row) {
      row.count++;
    });
  }

  // the purpose of this action is just to post a new deferred tx for the count action,
  // that might fail and should not break the chain of deferred tx of recurring_action.
  // this is an example of use to avoid interruptions on the recurring action calling, and
  // also as a safe way to prevent from having to pay CPU for actions that didn't do any
  // changes to state... this way the action can simply assert and the recurrence continues
  [[eosio::action]]
  void tick() {
    call_action(eosio::name("count"));
  }

  // NOTE: be careful with notifications on the main class, or any other base class,
  // as they might be called very often and can delay the execution of the recurring action,
  // because every time an action is called, a new deferred transaction is sent and the old
  // one is cancelled. It is advised to also call the recurring action on any other action,
  // perhaps by an inline action, if the desired is to execute "at most" with delay intervals.
  // Without that the standard behaviour is only deliver the delay without any contract
  // iteractions, so the delay is an "at least" interval.
  //
  // [[eosio::on_notify("eosio::onerror")]]
  // void onerror() {
  //   auto error = eosio::onerror::from_current_action();
  //
  //   eosio::print("we got an error for sender:", eosio::name(error.sender_id), error.sender_id);
  // }

protected:
  struct [[eosio::table]] profile_entry {
    eosio::name user;
    std::string nickname;
    std::string avatar;
    std::string website;
    std::string locale;
    std::string metadata;
    uint64_t count = 0;

    uint64_t primary_key() const { return user.value; }
    uint64_t by_count() const { return count; }
  };
  typedef eosio::multi_index<eosio::name("profiles"), profile_entry> profiles_table;

};
