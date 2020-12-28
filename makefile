all:
	g++ --std=c++11 -o decoder decoder.cpp
	./decoder panda.jpg

clean:
	rm jpg decoder