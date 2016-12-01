#ifndef KVSTORE_H
#define KVSTORE_H



#define KEY_CAST(A) *(KVStore::Key*)A
#define VALUE_CAST(A) *(KVStore::Value*)A

class KVStore {
private:
public:
	class Value {
public:
		char value[32];
	};

	class Key {
public:
		char key[16];
	};


	virtual bool insert(Key &key, Value &value) = 0;
	virtual bool find(Key &key, Value *out_value) = 0;
	virtual bool erase(Key &key) = 0;
	virtual bool update(Key &key, Value &value) = 0;
	virtual bool contains(Key &key) = 0;
	virtual size_t size() = 0;
	virtual ~KVStore(){};
};

inline bool operator == (const KVStore::Key & lhs, const KVStore::Key & rhs)
{
	return !memcmp(lhs.key, rhs.key, sizeof(KVStore::Key));
}

#endif /* KVSTORE_H */
