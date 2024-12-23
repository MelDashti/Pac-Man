#ifndef GHOST_H
#define GHOST_H

#include <stdbool.h>

// ghost structure

typedef struct {
	int row;
	int col;
	bool isChasing;
	bool isActive;
	int respawnTimer;
	int frightenedTimer;
	int underlyingCell;
	
}Ghost;

extern Ghost blinky;

// function declaration
void initGhost(void);
void updateGhost(void);
void drawGhost(int offsetX, int offsetY);

#endif