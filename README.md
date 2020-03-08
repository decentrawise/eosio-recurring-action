# eosio-recurring-action
A base class that makes an action recurring with a defined interval delay for EOSIO contracts and tests built with eoslime

## How it works

The only thing needed is to plug the recurring_action header file and the base class on your contract code, specifying the sender id for the deferred transaction, the delay between calls and the action you want to recur. Then your contract will have that action being called with **at least** that interval.

```
#include "recurring_action.hpp"

class [[eosio::contract]] profile: public eosio::contract, recurring_action {

  ...

public:

  profile(eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds) :
    contract(receiver, code, ds),
    recurring_action(receiver, RECURRING_SENDERID, RECURRING_DELAYSEC, RECURRING_ACTION)
  {}

  [[eosio::action]]
  void ...
```

With this, every time an action is called a new deferred transaction is sent and the old one is cancelled, if any, creating a recurring loop... Please refer to the included test contract for a complete working example.

It is advised to also call the recurring action on any other action, perhaps by an inline action, if the desired is to execute **at most** with delay intervals. Without that the standard behaviour is only deliver the delay without any contract iteractions, so the delay is an **at least** interval and might be more extended when contract iteractions occur.

**NOTE:** be careful with notifications on the main class, or any other contract class, as they might be called very often and can delay the execution of the recurring action.

## File tree

- src/ - source directory that contains `recurring_action.hpp` main project code file

- contracts/ - test contract directory

- tests/ - tests specs

- scripts/ - helper scripts for running nodeos locally

## Install EOSLime
```bash
$ npm install -g eoslime
```

## Compile the example contract
```bash
$ eoslime compile
```

## Run a local EOSIO node
```bash
$ ./scripts/nodeos.sh
```
**NOTE**: Please customize the script to your local development needs. This might be made
easier in the future with configuration and a better script...

## Run tests
```bash
$ eoslime test
```
