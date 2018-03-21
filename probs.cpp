#include <iostream>
#include <fstream>
#include <cmath>

using namespace std;

double attackerSuccess(int z ,double q){
	double p = 1.0 - q;
	double lambda = z * (q / p);
	double sum = 1.0;
	int i, k;
	for (k = 0; k <= z; k++){
		double poisson = exp(-lambda);
		for (i = 1; i <= k; i++){
			poisson *= lambda / i;
		}
		sum -= poisson * (1 - pow(q / p, z - k));
	}
	return sum;
}

int main(int argc, char** argv){
	ofstream ofs;
	ofs.open("attackerProb.csv", ofstream::out);
	ofs<<"Blocktime,Probability"<<endl;
	bool isUntil = false;
	int indOfNum = 0;
	int interval = 15;
	for(int i = 1; i<argc; i++){
		if(argv[i][0] == '-'){
			if(argv[i][1] == 'u'){
				isUntil = true;
			}
			if(argv[i][1] == 'i'){
				i++;
				interval = atoi(argv[i]);
			}
		}
		if(atoi(argv[i]) != 0 || atof(argv[i]) != 0.0){
			indOfNum = i;
		}
	}
	if(!isUntil){
		int bTimeLen = atoi(argv[indOfNum])/interval;
		int bTimes[bTimeLen];
		for(int i = 1; i<=bTimeLen; i++){
			bTimes[i-1] = i*interval;
		}

		for(const int& i : bTimes){
			double probSuccess = attackerSuccess(3600/i, 0.1);
			ofs<<i<<","<<probSuccess<<endl;
		}
	}
	else{
		double until = atof(argv[indOfNum]);
		double prob;
		int bTime = interval;
		do{
			prob = attackerSuccess(3600/bTime, 0.1);
			ofs<<bTime<<","<<prob<<endl;
			bTime+=interval;
		} while(prob < until);
	}
	ofs.close();
}
