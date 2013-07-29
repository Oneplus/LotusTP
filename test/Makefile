all: cws pos par

cws: cws.cpp
	g++ -o cws cws.cpp -I./ \
		-I../src/segmentor/ \
		-I../thirdparty/boost/include \
		-L../lib/ -lsegmentor -lboost_regex

pos: pos.cpp
	g++ -o pos pos.cpp -I./ -I../src/postagger/ -L../lib/ -lpostagger

par: par.cpp
	g++ -o par par.cpp -I./ -I../src/parser/ -L../lib -lparser

.PHONY: clean

clean:
	rm cws
	rm pos
	rm par
