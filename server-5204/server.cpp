#include "server.h"
#include "net.h"

int main(int argc, char **argv){

	NetworkServer *proxy = new NetworkServer();
	proxy->main_loop();

	return 0;
}

