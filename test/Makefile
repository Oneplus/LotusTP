all: cws pos

cws: cws.cpp
	g++ -o cws cws.cpp -I./ -I../src/segmentor/ -L../lib/ -lsegmentor

pos: pos.cpp
	g++ -o pos pos.cpp -I./ -I../src/postagger/ -L../lib/ -lpostagger

.PHONY: clean

clean:
	rm cws
	rm pos
