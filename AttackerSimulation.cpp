#include <math.h>
#include <iostream>
#include <iomanip>
#include <fstream>

double AttackerSuccessProbability(double q, double z){
    std::cout << "RUNNING WITH Z=" << z << std::endl;
    double p = 1.0 - q;
    double lambda = z * (q / p);
    double sum = 1.0;
    int i, k;
    for (k = 0; k <= z; k++)
    {
        double poisson = exp(-lambda);
        for (i = 1; i <= k; i++)
            poisson *= lambda / i;
        sum -= poisson * (1 - pow(q / p, z - k));
    }
    return sum;
}

int main(){
    const int target = 1200;
    std::ofstream data;
    data.open("attackerdata.csv");
    data << std::fixed << std::setprecision(20);
    data << "TIME,PROBABILITY" << std::endl;
    for(int i = 15; i <= 1200; i += 15){
        double prob = AttackerSuccessProbability(0.1, 3600/(double)i);
        if(prob < 0){
            prob = 0;
        }
        data << i << "," << prob << std::endl;
    }
    return 0;
}
