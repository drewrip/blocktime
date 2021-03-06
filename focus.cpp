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
#include <stdio.h>

#define trials 10

const int startTime = 10; // 10 Seconds
const int endTime =  60 * 10; // 10 Minutes

const std::string username = "user";
const std::string password = "password";
const std::string addr = "localhost"; // localhost
const int port = 2222; // Arbitrary port for specific testing
const std::string recp = "moRcYXywEzDXZAmnZypuv7SnjgLXGNKTep"; // Throwaway address
const double amt = 0.00001; // Small amount of bitcoin for rapid testing

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

	std::ofstream data;

	char * btcaddr;

	// Master node is opening data file
	if(nodeName == "master"){
		data.open("datafiles/timesdata.csv");
		data << "Time,TXs,Unconfirmed TXs\n";
	}

	std::ostringstream sstart;
	sstart << "/home/mpiuser/bitcoin/src/bitcoind -zapwallettxes=2 -daemon -server -listen -regtest -rpcpassword=password -rpcuser=user -rpcport=2222 -datadir=/home/mpiuser/bitcoin_" << nodeName << " -conf=/home/mpiuser/bitcoin_" << nodeName;

	// Starting bitcoin daemon with necessary settings
	system(sstart.str().c_str());
	std::cout << "Starting Bitcoin node on " << nodeName << std::endl;
	sleep(5);
	// Connecting with running bitcoin daemon
	BitcoinAPI client(username, password, addr, port);
	std::cout << nodeName << " connecting to the daemon @ " << addr << ":" << port << "..." << std::endl;
	sleep(1);

	MPI_Barrier(MPI_COMM_WORLD);

	if(worldRank == 0){
		// Generates blocks to fund wallets
		std::cout << "Generating test bitcoin" << std::endl;
		while(client.getbalance() < 6000){
			Value params;
			params.append(101);
			client.sendcommand("generate", params);
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if(worldRank == 0){
		for(int i = 0; i < 5; i++){
			MPI_Recv(&btcaddr[0], 35, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			std::string baddr(btcaddr);
			std::cout << "SENDING BITCOIN TO " << baddr << std::endl;
			client.sendtoaddress(baddr, 1000);
		}
	}
	else{
		std::cout << nodeName << ": Sent Address" << std::endl;
		MPI_Send(client.getnewaddress().c_str(), 35, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
	}
	MPI_Barrier(MPI_COMM_WORLD);
	if(worldRank == 0){
		Value pld;
		pld.append(101);
		client.sendcommand("generate", pld);
	}

	MPI_Barrier(MPI_COMM_WORLD);
	std::cout << nodeName << ": " << client.getbalance() << " BTC" << std::endl;
	MPI_Barrier(MPI_COMM_WORLD);

	// Runs time intervals
	for(int testTime = startTime; testTime <= endTime; testTime += 10){
		std::cout << nodeName << ": reached barrier" << std::endl;
		MPI_Barrier(MPI_COMM_WORLD);
		if(worldRank == 0){
			usleep(3000);
			std::cout << "                           STARTING TIME " << testTime << " sec" << std::endl;
			std::cout << "##########################################################################" << std::endl;
		}
		// Runs trials
		for(int i = 0; i < trials; i++){
			if(worldRank == 0){
				std::cout << "--- Running Trial " << i+1 << "/" << trials << " ---" << std::endl;
				std::cout << "UNCONF. TXs: " << client.getrawmempool().size() << std::endl;
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
			MPI_Barrier(MPI_COMM_WORLD);

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
				// Records the number of transactions not included in the block generated
				int unconf = client.getrawmempool().size();
				// Records the number of transactions in the generated block
				int txs = client.getblock(client.getbestblockhash()).tx.size() - 1;
				data << testTime << "," << txs << "," << unconf << "\n";
				std::cout << "BLOCKTIME: " << testTime << " sec - TRANSACTIONS: " << txs << " - UNCONFIRMED: " << unconf << std::endl;
			}
		}
		MPI_Barrier(MPI_COMM_WORLD);
		sleep(1);
		while(client.getrawmempool().size() > 0){
			Value opt;
			opt.append(1);
			client.sendcommand("generate", opt);
			sleep(1);
		}
	}

	if(nodeName == "master"){
		data.close();
		system("python3 datafiles/update.py");
	}
	MPI_Finalize();
	return 0;
}
