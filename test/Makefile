all: cws pos par ner

cws: cws.cpp
	g++ -o cws cws.cpp -I./ \
		-I../src/segmentor/ \
		-I../thirdparty/boost/include \
		-L../lib/ -lsegmentor -lboost_regex

pos: pos.cpp
	g++ -o pos pos.cpp -I./ -I../src/postagger/ -L../lib/ -lpostagger

ner: ner.cpp
	g++ -o ner ner.cpp -I./ -I../src/ner/ -L../lib -lner

par: par.cpp
	g++ -o par par.cpp -I./ -I../src/parser/ -L../lib -lparser

.PHONY: clean

clean:
	rm cws
	rm pos
	rm ner
	rm par
