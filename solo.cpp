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

#define trials 1

const int startTime = 30; // 30 Seconds
const int endTime =  20 * 60; // 20 Minutes

const std::string username = "user";
const std::string password = "password";
const std::string addr = "localhost"; // localhost
const int port = 2222; // Arbitrary port for specific testing
const std::string recp = "moRcYXywEzDXZAmnZypuv7SnjgLXGNKTep"; // Throwaway address
const double amt = 0.00001; // Small amount of bitcoin for rapid testing

int main(int argc, char * argv[]){

	int testTime = atoi(argv[1]);
	std::ofstream data;

	data.open("datafiles/timesdata.csv", std::ios::out | std::ios::app);

	std::ostringstream sstart;
	sstart << "/home/mpiuser/bitcoin/src/bitcoind -zapwallettxes=2 -daemon -server -listen -regtest -rpcpassword=password -rpcuser=user -rpcport=2222 -conf=/home/mpiuser/.bitcoin -datadir=/home/mpiuser/.bitcoin";

	// Starting bitcoin daemon with necessary settings
	system(sstart.str().c_str());
	std::cout << "Starting Bitcoin node on " << "Main" << std::endl;
	sleep(5);
	// Connecting with running bitcoin daemon
	BitcoinAPI client(username, password, addr, port);
	std::cout << "Main" << " connecting to the daemon @ " << addr << ":" << port << "..." << std::endl;
	sleep(1);

		Value params;
		params.append(1000);
		client.sendcommand("generate", params);

	std::cout << "Main" << ": " << client.getbalance() << " BTC" << std::endl;

	// Runs time intervals
		std::cout << "Main" << ": reached barrier" << std::endl;
		usleep(30000);
		std::cout << "                           STARTING TIME " << testTime << " sec" << std::endl;
		std::cout << "#################################################################" << std::endl;
			std::cout << "UNCONF. TXs: " << client.getrawmempool().size() << std::endl;
			// Sends transactions for duration of blocktime
			std::chrono::milliseconds ms(testTime * 1000);
			std::chrono::time_point<std::chrono::steady_clock> end;
			end = std::chrono::steady_clock::now() + ms;
			while(std::chrono::steady_clock::now() < end){
				try{
					client.sendtoaddress(recp, amt);
					std::cout << "Main" << " sent tx..." << std::endl;
				}
				catch(BitcoinException e){
					std::cout << "Main" << ": " << e.getMessage() << std::endl;
				}
			}

			Value opt;
			opt.append(1);
			client.sendcommand("generate", opt);
			// Records the number of transactions not included in the block generated
			int unconf = client.getrawmempool().size();
			// Records the number of transactions in the generated block
			int txs = client.getblock(client.getbestblockhash()).tx.size() - 1;
			data << testTime << "," << txs << "," << unconf << "\n";
			std::cout << "BLOCKTIME: " << testTime << " sec - TRANSACTIONS: " << txs << " - UNCONFIRMED: " << unconf << std::endl;

		sleep(1);
		while(client.getrawmempool().size() > 0){
			Value opt;
			opt.append(1);
			client.sendcommand("generate", opt);
		}
	data.close();
	return 0;
}
