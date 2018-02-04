#include <mpi.h>
#include <bitcoinapi/bitcoinapi.h>
#include <string>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>

const int start = 10; // 10 Seconds
const int end = 60 * 10; // 10 Minutes

const std::string username = "user";
const std::string password = "password";
const std::string addr = "localhost"; // localhost
const int port = 2222; // Arbitrary port for specific testing
const std::string recp = "miNcNbbqp39Th3QYkiqTXW2QauQ5JKcmjR"; // Throwaway address 
const double amt = 0.0001; // Small amount of bitcoin for rapid testing

const std::string hosts[6] = {"master", "node0", "node1", "node2", "node3", "node4"};

int main(){
	// Initialization Code for MPI specific utilities
	MPI_Init(NULL, NULL);

	// Stores the total number of processes in the network
	int worldSize;
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	// Stores the rank(identification # starting at 0) of the current process
	int worldRank;
	MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

	// Stores the name of the host currently running on
	char cnodeName[MPI_MAX_PROCESSOR_NAME];
	int namelen;
	MPI_Get_processor_name(cnodeName, &namelen);
	std::string nodeName(cnodeName);

	// Starting bitcoin daemon with necessary settings
	system("/home/mpiuser/bitcoin/src/bitcoind -daemon -server -listen -regtest -rpcpassword=password -rpcuser=user -rpcport=2222 -datadir=/home/mpiuser/bitcoin_" + nodeName);

	// Connecting with running bitcoin daemon
	BitcoinAPI client(username, password, addr, port);
	std::cout << nodeName << " connecting to the daemon @ " << addr << ":" << port << "..." << std::endl;
	sleep(1);

	MPI_Barrier(MPI_COMM_WORLD);

	// Adds other nodes
	for(std::string s : hosts){
		if(s != nodeName){
			client.addnode(nodeName, "add");
			std::cout << nodeName << " added " << s << std::endl;
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	// Generates blocks (Unlocks coinbase)
	Value params;
	params.append(101);
	client.sendcommand("generate", params);

	MPI_Barrier(MPI_COMM_WORLD);

	// Running time intervals
	for(int testTime = startTime; testTime <= endTime; testTime += 5){
		// Running 2 Trials
		for(int i = 0; i < 3; i++){
			MPI_Barrier(MPI_COMM_WORLD);
			std::cout << "Working on " << testTime << "sec..." << std::endl;
			auto start = chrono::steady_clock::now();
			while(chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now()-start).count() < 5000){
				client.sendtoaddress(recp, amt);
			}
			MPI_Barrier(MPI_COMM_WORLD);
			if(worldRank == 0){
				// Generates blocks (Unclocks coinbase)
				Value params;
				params.append(101);
				client.sendcommand("generate", params);
			}
			MPI_Barrier(MPI_COMM_WORLD);
		}
		if(worldRank == 0){

		}
		MPI_Barrier(MPI_COMM_WORLD);
	}	

	MPI_Finalize();
	return 0;
}