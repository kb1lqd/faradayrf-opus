all: opusenc opusdec

opusenc: opusenc.c
	gcc -o opusenc opusenc.c -lopus

opusdec: opusdec.c
	gcc -o opusdec opusdec.c -lopus
