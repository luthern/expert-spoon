#include "server.h"
#include "net.h"

int main(int argc, char **argv){

	NetworkServer *proxy = new NetworkServer();
	proxy->isrdma = '0';
	if(argc>1){
		proxy->isrdma = argv[1][0];
	}	
	proxy->main_loop();

	return 0;
}

