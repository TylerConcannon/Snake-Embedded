#ifndef FULLSNAKE_H
#define FULLSNAKE_H

#include <stdlib.h>
#include <iostream>
#include <cstdlib>

class FullSnake
{
public:
    FullSnake();

    struct SnakePart {
        public:
            int x;				//X-Horizontal coordinate
            int y;				//Y-Verticle coordinate
            struct SnakePart * next;		//To Next Body Part (Start of Body)
    };

    //Functions
    void moveSnek(int dir);
    SnakePart * getHead();
    SnakePart * getTail();
    void addPart(int dir);
    int getDir();
    void setDir(int dir);

    //Constants for direction
    const static int UP = 1;
    const static int LEFT = 2;
    const static int DOWN = 3;
    const static int RIGHT = 4;

private:

    int xStart = rand() % 44 + 2;
    int yStart = rand() % 23 + 2;

    int dir;

    SnakePart * Head = NULL;
    SnakePart * Tail = NULL;

};

#endif // FULLSNAKE_H
