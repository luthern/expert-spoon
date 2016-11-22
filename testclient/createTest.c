#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <random>


void help(char * name){
    printf("-n: The number of tests to generate (Default=50)\n");
    printf("-p: The probability that a read request will be made instead of write (Default=90) \n");
    printf("-k: The length of the keys to be generated (Default=16)\n");
    printf("-v: The length of the values to be generated (Default=32)\n");
    printf("-t: The title of the file to write the requests to (Default=test.txt)\n");
    printf("-c: The number of clients that will be running simultaneously and how many different test files to make(Default=1)\n");
    printf("-o: The offset to be added to the number at the end of the generated test file names(Default=0)\n");
    printf("-a: The alphabet of letters that the characters for the key and value will be picked from\nDefault=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ)\n");
    printf("-y: The minimum key length(Default=16)\n");
    printf("-z: The minimum value length(Default=32)\n");
    printf("-h: Displays this message\n");
}


int main(int argc, char* argv[])
{	
	srand(time(NULL));
	int option;
	int testcount = 50;
	int readProb = 90;
	int keyLength = 16;
	int valueLength = 32;
	int keyvariance = 0;
	int valuevariance = 0;
	int keyoffset = 0;
	int valueoffset = 0;
	int clientcount = 1;
	int offset = 0;
	char mode;
	std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::string title = "test";
	srand((unsigned) time(NULL));
	std::ofstream ofs;

    	while ((option = getopt(argc, argv,"h:n:k:v:p:t:c:o:a:y:z:")) != -1) {
        	switch (option) {
             		case 'n' :
                 		testcount = atoi(optarg);
                 		break;
             		case 'k' :
                 		keyLength = atoi(optarg);
                 		break;
             		case 'v' :
                 		valueLength = atoi(optarg);
                 		break;
             		case 'p' :
                 		readProb = atoi(optarg);
                 		break;
             		case 'c' :
                 		clientcount = atoi(optarg);
                 		break;
             		case 'o' :
                 		offset = atoi(optarg);
                 		break;
             		case 'y' :
                 		keyvariance = atoi(optarg);
                 		break;
             		case 'z' :
                 		valuevariance = atoi(optarg);
                 		break;
             		case 't' :
                 		title = std::string(optarg);
                 		break;
             		case 'a' :
                 		alphabet = std::string(optarg);
                 		break;
            		case 'h':
                		help(argv[0]);
                		exit(EXIT_FAILURE);
                		break;
             		default:
                		help(argv[0]);
                		exit(EXIT_FAILURE);
				break;
		}
    	}

	//Creates a different test file for each client
	for(int z = 0; z<clientcount; z++){
		std::stringstream stream;
		stream << z+offset;

		//Opens test file to read from
		ofs.open ((title+stream.str()+".txt").c_str(), std::ofstream::out | std::ofstream::trunc);

		//Writes the number of tests in the file at the top of the file
		ofs << testcount << "\n";
		keyoffset = keyLength - rand()%(keyLength-keyvariance+1);
		valueoffset = valueLength - rand()%(valueLength-valuevariance+1);
		for(int i = 0; i<testcount; i++){
			if(rand()%100+1<readProb)
			{
				ofs << "r:";
				mode = 'r';
			}
			else
			{
				ofs << "w:";
				mode = 'w';
			}
			for(int j = 0; j<keyoffset; j++){
				ofs << alphabet[rand()%52];
			}
			ofs << ":";
			if(mode=='w'){
				for(int k = 0; k<valueoffset; k++)
				{
					ofs << alphabet[rand()%52];
				}
				ofs << ":";
			}

			ofs << "\n";
		}
		ofs.close();
	}
}

