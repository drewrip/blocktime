#!/bin/bash

rm -rf .bitcoin
rm -rf /home/mpiuser/bitcoin_master/regtest
rm -rf /home/mpiuser/bitcoin_node0/regtest
rm -rf /home/mpiuser/bitcoin_node1/regtest
rm -rf /home/mpiuser/bitcoin_node2/regtest
rm -rf /home/mpiuser/bitcoin_node3/regtest
rm -rf /home/mpiuser/bitcoin_node4/regtest
sudo killall bitcoind
ssh node0 killall bitcoind
ssh node1 killall bitcoind
ssh node2 killall bitcoind
ssh node3 killall bitcoind
ssh node4 killall bitcoind
