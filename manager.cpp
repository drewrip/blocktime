#include <mpi.h>
#include <bitcoinapi/bitcoinapi.h>
#include <string>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <cstdlib>
#include <ctime>

#define trials 3

const int startTime = 10; // 10 Seconds
const int endTime = 60 * 10; // 10 Minutes

const std::string username = "user";
const std::string password = "password";
const std::string addr = "localhost"; // localhost
const int port = 2222; // Arbitrary port for specific testing
const std::string recp = "moRcYXywEzDXZAmnZypuv7SnjgLXGNKTep"; // Throwaway address 
const double amt = 0.0001; // Small amount of bitcoin for rapid testing

const std::string hosts[6] = {"master", "node0", "node1", "node2", "node3", "node4"};

void startd(std::string nname){
	std::ostringstream sstart;
	sstart << "/home/mpiuser/bitcoin/src/bitcoind -daemon -server -listen -regtest -rpcpassword=password -rpcuser=user -rpcport=2222 -datadir=/home/mpiuser/bitcoin_" << nname << " -conf=/home/mpiuser/bitcoin_" << nname;
	system(sstart.str().c_str());
	std::cout << nname << " connecting to the daemon @ " << addr << ":" << port << "..." << std::endl;
	sleep(1);
}

void stopd(){
	system("/home/mpiuser/bitcoin/src/bitcoin-cli -regtest -rpcpassword=password -rpcuser=user -rpcport=2222 stop");
}

void delMem(std::string nname){
	std::ostringstream sdel;
	sdel << "rm /home/mpiuser/bitcoin_" << nname << "/regtest/mempool.dat";
	system(sdel.str().c_str());
}

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

	std::ofstream data;

	// Master node is opening data file
	if(nodeName == "master"){
		data.open("timesdata.csv");
		data << "Time,TXs\n";
	}
	// Function starts daemon and connects to the api

	stopd();
	sleep(1);
	delMem(nodeName);
	sleep(1);
	startd(nodeName);
	sleep(1);
	BitcoinAPI client(username, password, addr, port);

	MPI_Barrier(MPI_COMM_WORLD);

	// Generates blocks (Unlocks coinbase)
	while(client.getbalance() < 50){
		Value params;
		params.append(101);
		client.sendcommand("generate", params);	
	}

	std::cout << nodeName << ": " << client.getbalance() << " BTC" << std::endl;
	MPI_Barrier(MPI_COMM_WORLD);

	// Runs block time intervals
	for(int testTime = startTime; testTime <= endTime; testTime += 10){
		MPI_Barrier(MPI_COMM_WORLD);
		if(worldRank == 0){
			usleep(3000);
			std::cout << "                           STARTING TRIAL - " << testTime << " sec" << std::endl;
			std::cout << "##########################################################################" << std::endl;
		}
		// Runs number of trials
		for(int i = 0; i < trials; i++){
			if(worldRank == 0){
				std::cout << "--- Running Trial " << i + 1 << "/" << trials << " ---"<< std::endl;
			}

			MPI_Barrier(MPI_COMM_WORLD);
			// Sends transactions for duration of blocktime
			std::chrono::milliseconds ms(testTime * 1000);
			std::chrono::time_point<std::chrono::steady_clock> end;
			end = std::chrono::steady_clock::now() + ms;
			while(std::chrono::steady_clock::now() < end){
				try{
					client.sendtoaddress(recp, amt);
					std::cout << nodeName << " sent tx..." << std::endl;
				}
				catch(BitcoinException e){
					std::cout << nodeName << ": " << e.getMessage() << std::endl;
				}
			}

			// Picks a random node to generate the block then informs the rest of the network
			int pick = 0;
			if(worldRank == 0){
				srand(time(NULL));
				pick = rand() % 6;
			}

			MPI_Bcast(&pick, 1, MPI_INT, 0, MPI_COMM_WORLD);

			MPI_Barrier(MPI_COMM_WORLD);

			// After blocktime as passed one node generates a new block
			if(worldRank == pick){
				Value opt;
				opt.append(1);
				client.sendcommand("generate", opt);
			}

			MPI_Barrier(MPI_COMM_WORLD);

			if(nodeName == "master"){
				// Records the number of transactions in the generated block
				int txs = client.getblock(client.getbestblockhash()).tx.size() - 1;
				data << testTime << "," << txs << "\n";
				std::cout << "BLOCKTIME: " << testTime << " sec - TRANSACTIONS: " << txs << std::endl; 
			}
		}
		client.stop();
		stopd();
		sleep(1);
		delMem(nodeName);
		sleep(1);
		startd(nodeName);
		sleep(1);
		BitcoinAPI client(username, password, addr, port);
	}

	if(nodeName == "master"){
		data.close();
	}
	MPI_Finalize();
	return 0;
}