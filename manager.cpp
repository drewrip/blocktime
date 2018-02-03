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
const std::string addr = "localhost"; // Localhost
const int port = 2222; // Default regtest port 
const std::string recp = "miNcNbbqp39Th3QYkiqTXW2QauQ5JKcmjR"; // Throwaway address 
const double amt = 0.0001; // Small amount of bitcoin for rapid testing

int main(){
	// Initialization Code for MPI specific utilities
	MPI_Init(NULL, NULL);

	// Stores the total number of processes in the network
	int worldSize;
	MPI_Comm_size(MPI_COMM_WORLD, &worldSize);

	// Stores the rank(identification # starting at 0) of the current process
	int worldRank;
	MPI_Comm_rank(MPI_COMM_WORLD, &worldRank);

	// Connecting with running bitcoin daemon
	BitcoinAPI client(username, password, addr, port);
	std::cout << "Node" << worldRank << " connecting to the daemon @ " << addr << ":" << port << "..." << std::endl;
	sleep(1);

	// Generates New Blocks (Unlocks coinbase)
	Value params;
	params.append(101);
	client.sendcommand("generate", params);



	MPI_Finalize();
}