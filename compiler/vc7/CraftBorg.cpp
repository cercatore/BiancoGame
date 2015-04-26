#define CRAFTBORG_CPP

#include "stdio.h"
#include "socket.h"

class GameSingleRecord     {
int gameId;
char  *  ipPla;
int status;
public:
Socket * l;

	GameSingleRecord() : gameId(0), ipPla(0) {
		ipPla = new char[15];
	}
		~GameSingleRecord(){}
		GameSingleRecord(int id, char * ip , int status_, Socket *  s) {
			ipPla = new char[15];
			status = status_;
			gameId = id;
			l = s;
			sprintf(ipPla,"%s",ip);
		}
};
