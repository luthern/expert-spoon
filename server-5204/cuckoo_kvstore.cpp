#include "cuckoo_kvstore.h"

bool CuckooKVStore::insert(KVStore::Key &key, Value &value)
{
	return hashtable.insert(key, value);
}

bool CuckooKVStore::find(KVStore::Key &key, Value *out_value)
{
	return hashtable.find(key, *out_value);
}

bool CuckooKVStore::erase(KVStore::Key &key)
{
	return hashtable.erase(key);
}

bool CuckooKVStore::contains(KVStore::Key &key)
{
	return hashtable.contains(key);
}

bool CuckooKVStore::update(KVStore::Key &key, Value &value)
{
	return hashtable.update(key, value);
}

size_t CuckooKVStore::size()
{
	return hashtable.size();
}
