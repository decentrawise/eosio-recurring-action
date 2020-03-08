// recurring actions for EOSIO smart contracts
#pragma once


#include <libc/bits/stdint.h>

#include <eosio/eosio.hpp>
#include <eosio/transaction.hpp>


#define EOSIO_MAX_DELAY 3888000


class recurring_action {

  // the contract owner account
  eosio::name owner;

  // the deferred tx id
  uint128_t sender_id;

  // the deferred tx delay in seconds
  unsigned int delay_sec;

  // the action to call
  eosio::name action;

  // sends a deferred tx calling our action
  void next() {
    // first remove the deferred tx if any
    eosio::cancel_deferred(sender_id);

    // then add new one
    eosio::transaction tx;
    tx.actions.emplace_back(
      eosio::permission_level(owner, eosio::name("active")),
      owner,
      action,
      0
    );
    tx.delay_sec = delay_sec <= EOSIO_MAX_DELAY ? delay_sec : EOSIO_MAX_DELAY;
    tx.send(sender_id, owner, false);
  }

protected:

  recurring_action(eosio::name owner, uint128_t sender_id, unsigned int delay_sec, eosio::name action) :
                   owner(owner), sender_id(sender_id), delay_sec(delay_sec), action(action) {
    // automatically send new deferred tx on all contract actions
    next();
  }

};
