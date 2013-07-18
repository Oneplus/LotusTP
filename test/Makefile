all: cws

cws: cws.cpp
	g++ -o cws cws.cpp -I./ -I../src/segmentor/ -L../lib/ -lsegmentor

.PHONY: clean

clean:
	rm cws
