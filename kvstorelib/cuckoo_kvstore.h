#ifndef CUCKOO_KVSTORE_H
#define CUCKOO_KVSTORE_H

#include "libcuckoo/cuckoohash_map.hh"
#include "libcuckoo/city_hasher.hh"

#include "kvstore.h"

class CuckooKVStore: public KVStore {
private:
	cuckoohash_map < KVStore::Key, KVStore::Value, CityHasher < KVStore::Key >> hashtable;
public:
	bool insert(KVStore::Key &key, KVStore::Value &value);
	bool find(KVStore::Key &key, KVStore::Value *out_value);
	bool erase(KVStore::Key &key);
	bool update(KVStore::Key &key, KVStore::Value &value);
	bool contains(KVStore::Key &key);
	size_t size();
	~CuckooKVStore();
};

#endif
