#include "debug.h"

#include "cstdarg"
#include <ctime>
#include <windows.h>

#ifdef _DEBUG

#define LOG_FILE "everest.txt"

#define MAX_LOG_LEN (10000)

static FILE * fileHandle = NULL;
static char logBuffer[MAX_LOG_LEN] = {0};

void initLogModule()
{
    fileHandle = fopen(LOG_FILE, "w+");
}

void exitLogModule()
{
	fclose(fileHandle);
	fileHandle = NULL;
}

void log(const char * fmt, ...){
    if (fileHandle == 0) return;
	va_list args;
	va_start(args, fmt);
	(void)vsnprintf(logBuffer, MAX_LOG_LEN, fmt, args);
	va_end(args);
	fprintf(fileHandle, "%s", logBuffer);
	fflush(fileHandle);
}
#else
void initLogModule()
{
}

void exitLogModule()
{
}

void log(const char * fmt, ...){
}	
#endif


unsigned long long GetMicroSec()
{
	static long long fq = 0;
	if (0 == fq)
	{
		QueryPerfomanceFrequency((LARGE_INTEGER *) &fq);
	}
	long long counter;
	QueryPerformanceCounter((LARGE_INTEGER *) &counter);
	return counter * 1000000 /fq;
	
}

void buildLegInfoFromMatrixMap(MatrixMap & matrix, Leg &leg)
{
	int col, row;
	Player player;
	int playerId = 0;
	
	for (int i=0; i < matrix._height * matrix._width; i++)
	{
		row = i/matrix._width;
		col = i % matrix._width;
		
		switch(matrix.getTerrain(col, row)) 
		{
			case E_FLAT:
				break; 
			case E_WALL:
			    leg._map_info._walls.push_back(Wall(col, row));
			    break;
		    case E_BRICK:
		        leg._map_info._bricks.push_back(Brick(col, row));
				break;
			case E_RIVER:
				leg._map_info._rivers.push_back(River(col,row));
		    case E_FOE:
		        player._pos.Set(col, row);
		        player._id = playerId++;
		        player._alive = true;
		        leg._map_info._enemy_players.push_back(player);
		        break;
			case E_FRIEND:
				player._pos.Set(col,row);
				player._id = playerId++;
				player._alive = true;
				leg._map_info._friend_players.push_back(player);
				break;		
		}
	}
	for (size_t i=0; i<matrix._diamondVec.size();i++)
    {
        leg._map_info._diamonds.push_back(Diamond(matrix._diamondVec[i].col, matrix._diamondVec[i].row));
    }

    for (size_t i=0; i<matrix._starVec.size(); i++)
    {
        leg._map_info._diamonds.push_back(Diamond(matrix._starVec[i].col, matrix._starVec[i].row));
    }
    return ;
}
