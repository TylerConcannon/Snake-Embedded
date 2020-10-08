#ifndef DATACACHE_H
#define DATACACHE_H

#include <semaphore.h>
#include <QMediaPlayer>
#include "snek.h"
#include "fullsnake.h"

class DataCache
{
public:
    DataCache();

    //Constants
    static const int SCREEN_WIDTH = 480;
    static const int SCREEN_HEIGHT = 270;
    static const int SCREEN_X = 25;
    static const int SCREEN_Y = 130;
    static const int BLOCK_WIDTH = 10;
    static const int BLOCK_HEIGHT = 10;

    static int pause_timer;

    //Mutexes
    static pthread_mutex_t direction_lock;
    static pthread_mutex_t snek_lock;
    static pthread_mutex_t pellet_lock;

    //Semaphore for starting threads
    static sem_t game_start_tokens;
    static sem_t move_snake_sem;
    static sem_t move_pellet_sem;
    static sem_t draw_screen_sem;

    //Media Player
    static QMediaPlayer* player;

    //Score
    static int score;

    //Game itself
    static Snek *game;

    //Game screen
    static QImage *imgScreen;

    //Snek for game
    static FullSnake* snek;

    //Pellet x & y
    static int pellet_x, pellet_y;

    //Eat flag & timestamp
    static int eaten;
    static QTime timestamp;

    //IT'S GAME OVER, MAN, GAME OVER
    static int game_over;

    //Direction lock - mutex was causing glitch
    static int dir_lock;

    //Game speed
    static int game_speed;

};

#endif // DATACACHE_H
