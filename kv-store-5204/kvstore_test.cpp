#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cuckoo_kvstore.h"

int main(int argc, char const *argv[])
{

	KVStore * kvstore = new CuckooKVStore();

	printf("sizeof(KVStore::Key): %lu\n", sizeof(KVStore::Key));
	printf("sizeof(KVStore::Key): %lu\n", sizeof(KVStore::Value));
	printf("Total elements in the kvstore: %lu\n", kvstore->size());
	// Insert some keys and values
	// Granted values are not 32 bytes,
	// but we are just printing the string out so they will be padded with junk
	printf("Inserting five items..\n");
	kvstore->insert(*(KVStore::Key*)"TES1TES1TES1TES1", *(KVStore::Value*)"VALUE1");
	kvstore->insert(*(KVStore::Key*)"TES2TES2TES2TES2", *(KVStore::Value*)"VALUE2");
	kvstore->insert(*(KVStore::Key*)"TES3TES3TES3TES3", *(KVStore::Value*)"VALUE3");
	kvstore->insert(*(KVStore::Key*)"TES4TES4TES4TES4", *(KVStore::Value*)"VALUE4");
	kvstore->insert(*(KVStore::Key*)"TES5TES5TES5TES5", *(KVStore::Value*)"VALUE5");
	printf("Total elements in the kvstore: %lu\n", kvstore->size());
	printf("Getting value for key 'TES3TES3TES3TES3'\n");
	KVStore::Value v;
	bool err = kvstore->find(*(KVStore::Key*)"TES3TES3TES3TES3", &v);
	printf("find() returned: %d\n", err);
	printf("Value contents: %s\n", (char*)&v);
	printf("Deleting pair for key 'TES3TES3TES3TES3'\n");
	err = kvstore->erase(*(KVStore::Key*)"TES3TES3TES3TES3");
	printf("Deletion returned: %d\n", err);
	printf("Checking whether the store contains 'TES3TES3TES3TES3'\n");
	err = kvstore->contains(*(KVStore::Key*)"TES3TES3TES3TES3");
	printf("contains() returned: %d\n", err);
	printf("Total elements in the kvstore: %lu\n", kvstore->size());

}
