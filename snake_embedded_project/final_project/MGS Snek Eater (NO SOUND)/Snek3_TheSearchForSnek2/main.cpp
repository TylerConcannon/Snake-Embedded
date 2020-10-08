#include "snek.h"
#include "unistd.h"
#include <QApplication>
#include <pthread.h>
#include <semaphore.h>
#include <datacache.h>
#include <QTime>
#include <QDir>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN

//Function headers
void *gui_thread(void *arg);
void pellet_thread(int sig, siginfo_t *si, void *uc);
void *snek_thread(void * arg);
void *mine_thread(void * arg);
void drawPixel(QImage *imgScreen, QColor color, int x, int y);

//Data for mines
std::vector<int> mine_x;
std::vector<int> mine_y;

//Flag to indicate game started
bool flag = 0;

//Timestamp
timer_t timer;
time_t timestamp_mine;

//Game volume
int game_volume = 100;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Snek w;
    w.show();

    //Flag for creating timer
    struct itimerspec ts;
    struct sigaction sa;
    struct sigevent se;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = pellet_thread;
    if (sigaction(SIG, &sa, NULL) == -1) {
        return -1;
    }

    se.sigev_notify = SIGEV_SIGNAL;
    se.sigev_value.sival_ptr = &timer;
    se.sigev_signo = SIG;

    if(timer_create(CLOCKID, &se, &timer) == -1) {
        return -2;
    }

    ts.it_value.tv_sec = 19;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 10;
    ts.it_interval.tv_nsec = 0;

    if(timer_settime(timer, 0, &ts, NULL) == -1) {
        return -3;
    }

    //DataCache::player = new QMediaPlayer();

    DataCache::game_over = 0;

    //Initialize mutexes
    pthread_mutex_init(&DataCache::direction_lock, NULL);
    pthread_mutex_init(&DataCache::snek_lock, NULL);
    pthread_mutex_init(&DataCache::pellet_lock, NULL);

    //Initialize semaphore
    sem_init(&DataCache::game_start_tokens, 0, 0);

    // launch your pthreads from here
    pthread_t update_gui_thread;
    pthread_t update_snek_thread;
    pthread_t add_mine_thread;

    //Launch threads
    pthread_create(&update_gui_thread, NULL, gui_thread, NULL);
    pthread_create(&update_snek_thread, NULL, snek_thread, NULL);
    pthread_create(&add_mine_thread,NULL,mine_thread,NULL);

    // this should be the last line
    return a.exec();
}



//Thread that moves the snek mathematically
void* snek_thread(void *arg) {

    while(true) {

        //Wait for game to start
        sem_wait(&DataCache::game_start_tokens);

        //Sleep for mercy
        usleep(8500000);

        while(!DataCache::game_over) {


            //MUTEX LOCKING
            pthread_mutex_lock(&DataCache::snek_lock);

            //Move snek

            //Get direction
            int dir = DataCache::snek->getDir();

            //GET HEAD & TAIL
            FullSnake::SnakePart *head = DataCache::snek->getHead();
            FullSnake::SnakePart *tail = DataCache::snek->getTail();

            int x, y;

            //Get pixel color of probe
            QColor probe = Qt::black;
            if(dir == FullSnake::RIGHT && head->x < 47) {
                probe = DataCache::imgScreen->pixelColor((head->x+1)*DataCache::BLOCK_WIDTH, head->y*DataCache::BLOCK_HEIGHT);
                x = head->x+1;
                y = head->y;
            }
            else if (dir == FullSnake::UP && head->y > 0) {
                probe = DataCache::imgScreen->pixelColor(head->x*DataCache::BLOCK_WIDTH, (head->y-1)*DataCache::BLOCK_HEIGHT);
                x = head->x;
                y = head->y-1;
            }
            else if (dir == FullSnake::DOWN && head->y < 26) {
                probe = DataCache::imgScreen->pixelColor(head->x*DataCache::BLOCK_WIDTH, (head->y+1)*DataCache::BLOCK_HEIGHT);
                x = head->x;
                y = head->y+1;
            }
            else if (dir == FullSnake::LEFT && head->x > 0) {
                probe = DataCache::imgScreen->pixelColor((head->x-1)*DataCache::BLOCK_WIDTH, head->y*DataCache::BLOCK_HEIGHT);
                x = head->x-1;
                y = head->y;
            }
            else {
                //Out of bounds, YOU LOSE
                DataCache::game_over = 1;
            }

            //If game not over, continue
            if(!DataCache::game_over) {

                //If probe is pellet, add, else, move
                if(probe == Qt::yellow) {

                    //Add snake part
                    DataCache::snek->addPart(dir);

                    //Set the flag
                    DataCache::eaten = 1;

                    //Adjust values
                    DataCache::score += 1;

                    //Set score label
                    DataCache::game->setScore(DataCache::score);

                    struct itimerspec ts;

                    ts.it_value.tv_sec = 10;
                    ts.it_value.tv_nsec = 0;
                    ts.it_interval.tv_sec = 10;
                    ts.it_interval.tv_nsec = 0;

                    timer_settime(timer, 0, &ts, NULL);

                }
                else if(probe == Qt::red) {
                    std::vector<int>::iterator it_x, it_y;
                    for(it_x = mine_x.begin(), it_y = mine_y.begin(); it_x != mine_x.end(); it_x++, it_y++) {
                        if(x == *it_x && y == *it_y) {
                            it_x = mine_x.erase(it_x);
                            it_y = mine_y.erase(it_y);
                            break;
                        }
                    }

                    DataCache::score = DataCache::score / 2;

                    DataCache::game->setScore(DataCache::score);

                    DataCache::snek->moveSnek(dir);
                }
                else if(probe == Qt::green) {
                    //YOU LOSE
                    DataCache::game_over = 1;

                }
                else {
                    //UPDATE POSITION
                    DataCache::snek->moveSnek(dir);
                }

                DataCache::dir_lock = 0;
            }

            //MUTEX UNLOCKING
            pthread_mutex_unlock(&DataCache::snek_lock);

            if(DataCache::eaten == 1) {
                pellet_thread(NULL, NULL, NULL);
            }

            usleep(DataCache::game_speed);
        }
    }

    pthread_exit(NULL);
}

//Draw a dot on the screen given the coordinates
void drawPixel(QImage *imgScreen, QColor color, int x, int y) {

    //Scale up x and y
    x *= DataCache::BLOCK_WIDTH;
    y *= DataCache::BLOCK_HEIGHT;

    //Check if null
    if(DataCache::imgScreen != NULL) {
        //Draw the dot
        for (int i = x; i < x + DataCache::BLOCK_WIDTH; i++){
            for (int j = y; j < y + DataCache::BLOCK_HEIGHT; j++){
                DataCache::imgScreen->setPixelColor(i,j,color);
            }
        }
    }
}

//Thread that draws the screen
void* gui_thread(void *arg) {

    while(true) {

        //Wait for game to start
        sem_wait(&DataCache::game_start_tokens);

        while(!DataCache::game_over) {

            //MUTEX LOCKING
            pthread_mutex_lock(&DataCache::snek_lock);

            //DRAW SCREEN
            if(DataCache::imgScreen != NULL) {
                delete(DataCache::imgScreen);
            }

            DataCache::imgScreen = new QImage(480, 270, QImage::Format_RGB888);
            DataCache::imgScreen->fill(QColor(Qt::black).rgb());

            //Draw snek
            //Get the tail
            FullSnake::SnakePart *current = DataCache::snek->getTail();

            //Draw the rest of the body
            while(current != NULL) {
                drawPixel(DataCache::imgScreen, Qt::green, current->x, current->y);
                current = current->next;
            }

            //Get the head
            FullSnake::SnakePart *head = DataCache::snek->getHead();

            //Draw the head
            drawPixel(DataCache::imgScreen, Qt::darkGreen, head->x, head->y);

            //Draw pellet
            drawPixel(DataCache::imgScreen, Qt::yellow, DataCache::pellet_x, DataCache::pellet_y);

            //Draw the mine
            for(int i = 0; i < mine_x.size(); i++) {
                drawPixel(DataCache::imgScreen, Qt::red, mine_x[i], mine_y[i]);
            }

            //Update screen
            DataCache::game->updateScreen();

            //MUTEX UNLOCKING
            pthread_mutex_unlock(&DataCache::snek_lock);

            if(!flag) {

                //Play sound
//                QString endpath = "/codec.mp3";
//                DataCache::player->setMedia(QUrl::fromLocalFile(QDir::currentPath() + endpath));
//                DataCache::player->setVolume(100);
//                DataCache::player->play();

                //Sleep for sound
                usleep(2500000);

                //Display screen
                DataCache::game->displayIntroduction();

                //Sleep for screen display
                usleep(5000000);

                game_volume = 100;

                //Set flag
                flag = 1;
            }

            //Sleep for 10ms
            usleep(10000);

        }

//        QString endpath = "/game_over_audio.mp3";
//        DataCache::player->setMedia(QUrl::fromLocalFile(QDir::currentPath() + endpath));
//        DataCache::player->setVolume(game_volume);
//        DataCache::player->play();

        game_volume = 0;

        usleep(2500000);

        //Reset flag
        flag = 0;

        DataCache::game->lose();

    }

    pthread_exit(NULL);
}

//Thread that adds another mine
void* mine_thread(void *arg) {

    while(true) {

        //Wait for game to start
        sem_wait(&DataCache::game_start_tokens);

        //Sleep for mercy
        usleep(8500000);

        time(&timestamp_mine);

        while(!DataCache::game_over) {

            //MUTEX LOCKING
            pthread_mutex_lock(&DataCache::snek_lock);

            //Move pellet if snek eats or 10 seconds elapse
            time_t current_time;
            time(&current_time);

            if(difftime(current_time,timestamp_mine) >= 30) {

                //If there aren't 5 mines
                if(mine_x.size() < 10) {

//                    QString endpath = "/exclamation.mp3";
//                    DataCache::player->setMedia(QUrl::fromLocalFile(QDir::currentPath() + endpath));
//                    DataCache::player->setVolume(game_volume);
//                    DataCache::player->play();
                    usleep(10000);

                    int x, y;

                    do {
                        //RANDOMIZE LOCATION OF PELLET
                        x = rand()%48;
                        y = rand()%27;
                    } while(DataCache::imgScreen->pixelColor(x*DataCache::BLOCK_WIDTH,y*DataCache::BLOCK_HEIGHT) == Qt::green || DataCache::imgScreen->pixelColor(x*DataCache::BLOCK_WIDTH,y*DataCache::BLOCK_HEIGHT) == Qt::darkGreen);

                    mine_x.push_back(x);
                    mine_y.push_back(y);

                }

                //Reset timestamp
                time(&timestamp_mine);

            }

            //MUTEX UNLOCKING
            pthread_mutex_unlock(&DataCache::snek_lock);

        }

    }
    pthread_exit(NULL);

}

//Thread that randomized pellet location
void pellet_thread(int sig, siginfo_t *si, void *uc) {

            //MUTEX LOCKING
//            pthread_mutex_lock(&DataCache::snek_lock);

//                if(!DataCache::eaten) {
//                    QString endpath = "/exclamation.mp3";
//                    DataCache::player->setMedia(QUrl::fromLocalFile(QDir::currentPath() + endpath));
//                    DataCache::player->setVolume(game_volume);
//                    DataCache::player->play();
//                    usleep(10000);
//                }

                do {
                    //RANDOMIZE LOCATION OF PELLET
                    DataCache::pellet_x = rand()%48;
                    DataCache::pellet_y = rand()%27;
                } while(DataCache::imgScreen->pixelColor(DataCache::pellet_x*DataCache::BLOCK_WIDTH,DataCache::pellet_y*DataCache::BLOCK_HEIGHT) == Qt::green || DataCache::imgScreen->pixelColor(DataCache::pellet_x*DataCache::BLOCK_WIDTH,DataCache::pellet_y*DataCache::BLOCK_HEIGHT) == Qt::darkGreen);

                //Reset flag
                DataCache::eaten = 0;

            //MUTEX UNLOCKING
//            pthread_mutex_unlock(&DataCache::snek_lock);

}
