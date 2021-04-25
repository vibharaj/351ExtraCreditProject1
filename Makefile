
all: receiver sender

receiver: recvsignal.cpp
	g++ -o rSig recvsignal.cpp

sender: sendersignal.cpp
	g++ -o sSig sendersignal.cpp
