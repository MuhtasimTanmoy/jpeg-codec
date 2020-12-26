all:
	g++ --std=c++11 -o decoder decoder.cpp

clean:
	rm jpg decoder