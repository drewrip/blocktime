#include <bitcoinapi/bitcoinapi.h>
#include <string>

const std::string username = "rdsf";
const std::string password = "password";
const std::string addr = "127.0.0.1"; // Localhost
const int port = 18444; // Default regtest port 
const std::string recp = "miNcNbbqp39Th3QYkiqTXW2QauQ5JKcmjR"; // Throwaway address 
const double amt = 0.0001; // Small amount of bitcoin for rapid testing
	
int main(){

	try{
		BitcoinAPI client(username, password, addr, port);
		//client.sendcommmand("generate", 101);
	
		client.sendtoaddress(recp, amt);
	}
	catch(BitcoinException e){
		std::cerr << e.getMessage() << std::endl;
	}
}