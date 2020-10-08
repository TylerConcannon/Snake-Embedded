#include "fullsnake.h"
#include <stdlib.h>
#include <iostream>
#include <cstdlib>

void FullSnake::moveSnek(int dir)
{
        FullSnake::SnakePart * temp = new FullSnake::SnakePart; // POTENTIAL SOURCE OF ERROR

        addPart(dir);

        temp = Tail;
        Tail = Tail->next;
        delete(temp);
}

void FullSnake::addPart(int dir){
        FullSnake::SnakePart * temp = new FullSnake::SnakePart; // POTENTIAL SOURCE OF ERROR

        if (dir == UP) {
            temp->x = Head->x;
            temp->y = Head->y - 1;
        }

        else if (dir == LEFT) {
            temp->x = Head->x - 1;
            temp->y = Head->y;
        }

        else if (dir == DOWN) {
            temp->x = Head->x;
            temp->y = Head->y + 1;
        }

        else if (dir == RIGHT) {
            temp->x = Head->x + 1;
            temp->y = Head->y;
        }
        temp->next = NULL;
        Head->next = temp;
        Head = temp;
    }

FullSnake::FullSnake() {			//Constructor
        dir = rand() % 4 + 1;
        FullSnake::SnakePart * sHead = new FullSnake::SnakePart;
        sHead->x = xStart;
        sHead->y = yStart;
        sHead->next = NULL;

        FullSnake::SnakePart * bOne = new FullSnake::SnakePart;
        FullSnake::SnakePart * bTwo = new FullSnake::SnakePart;

        if (dir == UP) {		//up, body points down
            bOne->x = sHead->x;
            bOne->y = sHead->y - 1;
            bOne->next = sHead;
            dir = DOWN;
        }
        else if (dir == LEFT) {		//left, body points right
            bOne->x = sHead->x - 1;
            bOne->y = sHead->y;
            bOne->next = sHead;
            dir = RIGHT;
        }
        else if (dir == DOWN) {		//down, body points up
            bOne->x = sHead->x;
            bOne->y = sHead->y + 1;
            bOne->next = sHead;
            dir = UP;
        }

        else if (dir == RIGHT) {		//right, body points left
            bOne->x = sHead->x + 1;
            bOne->y = sHead->y;
            bOne->next = sHead;
            dir = LEFT;
        }

        do {
            int dir2 = rand()%4 + 1;

            if (dir2 == UP) {		//up, body points down
                bTwo->x = bOne->x;
                bTwo->y = bOne->y - 1;
                bTwo->next = bOne;
            }
            else if (dir2 == LEFT) {		//left, body points right
                bTwo->x = bOne->x - 1;
                bTwo->y = bOne->y;
                bTwo->next = bOne;
            }
            else if (dir2 == DOWN) {		//down, body points up
                bTwo->x = bOne->x;
                bTwo->y = bOne->y + 1;
                bTwo->next = bOne;
            }

            else if (dir2 == RIGHT) {		//right, body points left
                bTwo->x = bOne->x + 1;
                bTwo->y = bOne->y;
                bTwo->next = bOne;
            }

        } while(bTwo->x == sHead->x && bTwo->y == sHead->y);

        Head = sHead;
        Tail = bTwo;


}

FullSnake::SnakePart * FullSnake::getHead() {
    return Head;
}

FullSnake::SnakePart * FullSnake::getTail() {
    return Tail;
}

int FullSnake::getDir() {
    return dir;
}

void FullSnake::setDir(int dir) {
    this->dir = dir;
}


