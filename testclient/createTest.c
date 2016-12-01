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
#include <algorithm>

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
    printf("-l: Limit the keys and values made to those in a specific file");
    printf("-f: Failure occurance rate (trying to read something you never inserted, etc)\n");
    printf("-s: Specify the starting character for every key the client creates\n");
    printf("-w: The number of write operations to be done before a read operation is allowed (so the kv store gets properly filled) \nDefault=10\n");
    printf("-u: The probability of updating an old entry instead of writing to a new one (Default=50)\n");
    printf("-d: The probability of deleting an entry if you have chosen not to read (Default=10)\n");
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
	int opcode;
	std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	std::string title = "test";
	srand((unsigned) time(NULL));
	int limit = 0;
	std::string limitfilename;
	std::ofstream ofs;
	std::ifstream limitofs;
	std::string startchar;
	int usestartchar = 0;
	int failurerate = 0;
	int writeamount = 1;
	int updatechance = 50;
	int deletechance = 10;
	std::vector<std::string> *safekeys = new std::vector<std::string>();

    	while ((option = getopt(argc, argv,"h:n:k:v:p:t:c:o:a:y:z:l:f:s:w:u:d:")) != -1) {
        	switch (option) {
             		case 'n' :
                 		testcount = atoi(optarg);
                 		break;
             		case 'u' :
                 		updatechance = atoi(optarg);
                 		break;
             		case 'd' :
                 		deletechance = atoi(optarg);
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
             		case 'w' :
                 		writeamount = atoi(optarg);
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
             		case 'f' :
                 		failurerate = atoi(optarg);
                 		break;
             		case 't' :
                 		title = std::string(optarg);
                 		break;
             		case 'a' :
                 		alphabet = std::string(optarg);
                 		break;
             		case 'l' :
				limit = 1;
                 		limitfilename = std::string(optarg);
                 		break;
             		case 's' :
				usestartchar = 1;
                 		startchar = std::string(optarg);
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
	std::vector<std::string> *limitvector = new std::vector<std::string>();
	std::string line;
	int length;
	if(limit==1){
		limitofs.open(limitfilename);
		getline(limitofs, line);
		length = stoi(line);
		for(int r=0; r<length; r++)
		{
			getline(limitofs, line);
			limitvector->push_back(line);
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
		keyoffset = keyLength - rand()%(keyvariance+1);
		valueoffset = valueLength - rand()%(valuevariance+1);

			for(int i = 0; i<testcount; i++){
				if(rand()%100+1<readProb and safekeys->size()>=writeamount)
				{
					mode = 'r';
					opcode = 0;
				}
				else
				{
					if(rand()%100>deletechance)
					{
						mode = 'w';
						opcode = 1;
					}
					else
					{
						mode = 'd';
						opcode = 2;
					}
				}
				std::string keystring;
				if(limit==1)
				{
					int randomindex = rand()%(int)limitvector->size();	
					keystring = limitvector->at(randomindex);;
				}
				else
				{
					keystring = "";
					for(int j = 0; j<keyoffset; j++){
						if(usestartchar==1 and j==0)
						{
							keystring+=startchar;
						}
						else
						{
							keystring+=alphabet[rand()%alphabet.size()];
						}
					}
				}

				//Failure check for read (are you reading from a key that doesn't exist?)
				if(mode=='r')
				{
					//If the command could fail, then either it is changed to a write
					//or it is allowed to happen, based on failurerate
					if(std::find(safekeys->begin(), safekeys->end(), keystring) == safekeys->end())
					{
						if(rand()%100>=failurerate)
						{

							//If there are already known keys in the KV store then it will read from
							//a safe key or switch to write based on the read probability argument
							if(safekeys->size()>0)
							{
								if(rand()%100<readProb)
								{
									keystring = std::string(safekeys->at(rand()%safekeys->size()));
								}
								else
								{
									mode = 'w';
								}
							}
							//If there are no known keys in the KV store then the only non failure thing to do
							//is write
							else
							{
								mode='w';
							}
						}
					}
				}

				//Failure check for delete (am I trying to delete a key that doesn't exist?)
				if(mode=='d')
				{
					opcode = 2;
					//If the command could fail, then either it is changed to a write
					//or it is allowed to happen, based on failurerat
					if(std::find(safekeys->begin(), safekeys->end(), keystring) == safekeys->end())
					{
						if(rand()%100<70 and safekeys->size()>0)
						{
							keystring = std::string(safekeys->at(rand()%safekeys->size()));
							safekeys->erase(std::remove(safekeys->begin(), safekeys->end(), keystring), safekeys->end());
						}
						else
						{
							mode = 'w';
						}
					}
					else
					{
						safekeys->erase(std::remove(safekeys->begin(), safekeys->end(), keystring), safekeys->end());
					}
				}

				//Failure check for write (makes sure insert or update are used properly, unless failurerate>0
				if(mode=='w')
				{
					//Changes to insert or update depending on failure rate
					if(std::find(safekeys->begin(), safekeys->end(), keystring) == safekeys->end())
					{
						if(rand()%100>=failurerate)
						{
							if(rand()%100<=updatechance and safekeys->size()>writeamount)
							{
								mode = 'w';
								opcode = 3;
								keystring = std::string(safekeys->at(rand()%safekeys->size()));
							}
							else
							{	
								mode='w';
								opcode = 1;
								safekeys->push_back(keystring);
							}
						}
						else
						{
							if(rand()%100>updatechance)
							{
								mode='w';
								opcode = 1;
								keystring = std::string(safekeys->at(rand()%safekeys->size()));
							}
							else	
							{
								mode='w';
								opcode = 3;
							}
						}
						
					}
					else
					{
						mode='w';
						opcode = 3;
					}
				}
				ofs << opcode;
				ofs << ":";
				ofs << keystring;
				ofs << ":";
				std::string valuestring = "";
				if(mode=='w'){
					for(int k = 0; k<valueoffset; k++)
					{
						valuestring+=alphabet[rand()%alphabet.size()];
					}
					ofs << valuestring;
					ofs << ":";
				}

				ofs << "\n";
			}

	}
}

