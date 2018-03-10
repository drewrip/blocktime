#!/bin/bash

echo "Time,TXs,Unconfirmed TXs" > /home/mpiuser/blocktime/datafiles/timesdata.csv
COUNTER=1200
while [  $COUNTER -lt 100000 ]; do
	let COUNTER=COUNTER+50
	./solo $COUNTER
	sudo killall bitcoind
	rm -rf /home/mpiuser/.bitcoin/regtest
done
python3 /home/mpiuser/blocktime/datafiles/update.py
