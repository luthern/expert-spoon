#include "server.h"
#include "net.h"
#include "cuckoo_kvstore.h"

KVStore *kvstore;

int main(int argc, char **argv)
{
	kvstore = new CuckooKVStore();
	NetworkServer *proxy = new NetworkServer();

	proxy->main_loop();

	return 0;
}
