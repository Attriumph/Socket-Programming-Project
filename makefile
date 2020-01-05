all:
	g++ -std=c++11 client.cpp -o client
	g++ -std=c++11 aws.cpp -o aws1
	g++ -std=c++11 serverA.cpp -o serverA1
	g++ -std=c++11 serverB.cpp -o serverB1 -lm

serverA:
	./serverA1
serverB:
	./serverB1
aws:
	./aws1
client:
	./client1


