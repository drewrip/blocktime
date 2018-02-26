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
const int endTime =  10 * 60; // 10 Minutes

const std::string username = "user";
const std::string password = "password";
const std::string addr = "localhost"; // localhost
const int port = 2222; // Arbitrary port for specific testing
const std::string recp = "moRcYXywEzDXZAmnZypuv7SnjgLXGNKTep"; // Throwaway address
const double amt = 0.00001; // Small amount of bitcoin for rapid testing

int main(){

	std::ofstream data;

	data.open("timesdata.csv");
	data << "Time,TXs,Unconfirmed TXs\n";

	std::ostringstream sstart;
	sstart << "/home/mpiuser/bitcoin/src/bitcoind -zapwallettxes=2 -daemon -server -listen -regtest -rpcpassword=password -rpcuser=user -rpcport=2222";

	// Starting bitcoin daemon with necessary settings
	system(sstart.str().c_str());
	std::cout << "Starting Bitcoin node on " << "Main" << std::endl;
	sleep(5);
	// Connecting with running bitcoin daemon
	BitcoinAPI client(username, password, addr, port);
	std::cout << "Main" << " connecting to the daemon @ " << addr << ":" << port << "..." << std::endl;
	sleep(1);


	// Generates blocks (Unlocks coinbase)
	while(client.getbalance() < 500){
		Value params;
		params.append(101);
		client.sendcommand("generate", params);
	}

	std::cout << "Main" << ": " << client.getbalance() << " BTC" << std::endl;

	// Runs time intervals
	for(int testTime = startTime; testTime <= endTime; testTime += 10){
		std::cout << "Main" << ": reached barrier" << std::endl;
		usleep(3000);
		std::cout << "                           STARTING TIME " << testTime << " sec" << std::endl;
		std::cout << "##########################################################################" << std::endl;
		// Runs trials
		for(int i = 0; i < trials; i++){
			std::cout << "--- Running Trial " << i+1 << "/" << trials << " ---" << std::endl;
			int startUnconf;
			while(true){
				try{
					startUnconf = client.getrawmempool().size();
					break;
				}
				catch(BitcoinException e){
				}
			}
			std::cout << "UNCONF. TXs: " << startUnconf << std::endl;
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
		
		}
		system("/home/mpiuser/bitcoin/src/bitcoin-cli -regtest -rpcport=2222 -rpcuser=user -rpcpassword=password stop");
		std::ostringstream sremove;
		sleep(1);
		sremove << "/home/mpiuser/.bitcoin/regtest/mempool.dat";
		std::cout << "Clearing mempool..." << std::endl;
		remove(sremove.str().c_str());
		sleep(1);
		std::cout << "Restarting nodes..." << std::endl;
		system(sstart.str().c_str());
		sleep(5);
	}

	data.close();
	system("python3 datafiles/update.py");
	return 0;
}