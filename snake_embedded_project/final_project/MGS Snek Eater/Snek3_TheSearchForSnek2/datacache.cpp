#include "datacache.h"
#include "fullsnake.h"
#include <semaphore.h>
#include <QMediaPlayer>
#include <QTime>

DataCache::DataCache()
{

}

sem_t DataCache::game_start_tokens;
sem_t DataCache::move_snake_sem;
sem_t DataCache::move_pellet_sem;
sem_t DataCache::draw_screen_sem;

FullSnake* DataCache::snek;

int DataCache::pause_timer;
int DataCache::pellet_x;
int DataCache::pellet_y;
int DataCache::game_speed;
int DataCache::score;
int DataCache::eaten;
int DataCache::game_over;
int DataCache::dir_lock;

QTime DataCache::timestamp;

QMediaPlayer* DataCache::player;

const int SCREEN_WIDTH = 480;
const int SCREEN_HEIGHT = 270;
const int SCREEN_X = 25;
const int SCREEN_Y = 130;
const int BLOCK_WIDTH = 10;
const int BLOCK_HEIGHT = 10;

Snek* DataCache::game;

QImage* DataCache::imgScreen;

pthread_mutex_t DataCache::direction_lock;
pthread_mutex_t DataCache::snek_lock;
pthread_mutex_t DataCache::pellet_lock;
