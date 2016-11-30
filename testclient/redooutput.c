#include <stdio.h>
#include <stdint.h>
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
#include <string>
#include <sstream>
#include <unordered_map>

//Writes a new output file that combines the lines of processes that were split up by
//"unifinished" or "resumed" in the default strace output
//Makes the file easier to parse in by checkoutput.c
int main(int argc, char* argv[])
{
	std::string outputfilename = "output.txt";
	std::string newoutputfilename = "output2.txt";
	if(argc>1) outputfilename = std::string(argv[1]);
	if(argc>2) newoutputfilename = std::string(argv[2]);
	std::ifstream ifs;
	std::ofstream ofs;
	ifs.open(outputfilename.c_str());
	ofs.open(newoutputfilename.c_str(), std::ofstream::out | std::ofstream::trunc);
	std::string line;
	std::string pid;
	std::unordered_map<std::string, std::string> *hashmap = new std::unordered_map<std::string, std::string>();

	//Reads each line from the strace output file
	//If a sendto or recvfrom system call is found then it is written into the hashmap with the process pid as the key
	//Once the entire system call has been read from the file it is written to the new file as a single line
	while(std::getline(ifs, line))
	{
		if(line.find("sendto") != std::string::npos or line.find("recvfrom") != std::string::npos
			or line.find("send") != std::string::npos or line.find("recv") != std::string::npos)
		{
			std::istringstream linestream(line);
			linestream >> pid;
			std::string line2 = "";
			if(hashmap->find(pid) != hashmap->end()) line2 = hashmap->at(pid);
			std::string token;
			std::string token2 = "";
			if(line.find("resumed>") != std::string::npos)
			{
				linestream >> token;
				linestream >> token;
				linestream >> token;
			}
			linestream >> token;
			while(token!=token2 and token.find("unfinished") == std::string::npos)
			{
				//std::cout << line2 << "\n";
				line2.append(" " + token);
				token2 = token;
				linestream >> token;
			}
			if(token.find("unfinished") != std::string::npos)
			{
				if(hashmap->find(pid) != hashmap->end())
				{
					hashmap->find(pid)->second = line2;
				}
				else
				{
					hashmap->insert(std::make_pair(pid, line2));
				}
			}
			else
			{
				if(hashmap->count(pid) != 0)
				{
					hashmap->find(pid)->second = "";
				}
				ofs << pid + " " + line2 << "\n";
			}
		}
	}
	ifs.close();
	ofs.close();
}
