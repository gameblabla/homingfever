#ifndef _GAME_H_
#define _GAME_H_

#include "helpers.h"
#include "objects.h"

#define LEVEL_W			1600
#define LEVEL_H			1600

#define PLAYER_SPEED		2
#define PLAYER_ROTATION		4
#define PLAYER_COOLDOWN_TIME	(60*2)
#define PLAYER_PENALTY_TIME	(60*8)
#define GAME_OVER_TIME		(60*3)

extern listElement *objListHead;
extern object *playerObj;
extern tileset marker;
extern int gameTicks;
extern int gameTime;
extern int bestTime;
extern int playerLastAngle;
extern int playerPenaltyTimer;
extern int gameOverTimer;

void gameUnload();
void gameLoad();
void gameLogic();
void gameDraw();

#endif /* _GAME_H_ */
