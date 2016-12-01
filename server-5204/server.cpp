#include "server.h"
#include "net.h"
#include "kvstorelib.h"


// static void load_kvstore_from_file(char *fname)
// {
//      size_t kvpair_size = sizeof(KVStore::Key) + sizeof(KVStore::Value);
//      char kvpair[kvpair_size];
//      FILE *ptr;
//
//      ptr = fopen(fname, "rb");
//      while (fread(kvpair, kvpair_size, 1, ptr) == kvpair_size) {
//              char *key = kvpair;
//              char *value = kvpair + (sizeof(KVStore::Key));
//              kvstore->insert(KEY_CAST(key), VALUE_CAST(value));
//      }
//
// }
//
// static void generate_random_store(char *fname)
// {
//      FILE *ptr;
//
//      ptr = fopen(fname, "w");
//      for (size_t i = 0; i < 0x1000; i++) {
//              char key[16];
//              char value[32];
//              for (int j = 0; j < 16; j++)
//                      key[j] = rand() % 256;
//              for (int k = 0; k < 32; k++)
//                      value[k] = rand() % 256;
//              fwrite(key, 16, 1, ptr);
//              fwrite(value, 32, 1, ptr);
//      }
//      fclose(ptr);
// }

int main(int argc, char **argv)
{
	// generate_random_store("store.bin");
	kvstore_create();
	NetworkServer *proxy = new NetworkServer();

	proxy->main_loop();

	return 0;
}
