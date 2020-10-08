#ifndef SNEK_H
#define SNEK_H
#include<QPainter>
#include<QPainterPath>
#include<QMainWindow>
#include<semaphore.h>
#include "fullsnake.h"

namespace Ui {
class Snek;
}

class Snek : public QMainWindow
{
    Q_OBJECT

public:
    explicit Snek(QWidget *parent = 0);

    //Constants
    const int EASY_MODE = 500000;
    const int MED_MODE = 200000;
    const int HARD_MODE = 75000;
    const int EE_MODE = 20000;

    //Drawing functions
    void updateScreen();
    void displayIntroduction();
    void setScore(int value);
    void lose();

    ~Snek();

private slots:
    //Events for game control
    void createNewGame();
    void togglePause();
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();

public slots:
    //Slot for qDialog
    void computeDifficulty(const QString &);

protected:
    bool event(QEvent* event);


private:
    Ui::Snek *ui;

    //Flags
    int game_over;
    int paused = 0;

    //Variables
    int score = 0;              //score of the game

};

#endif // SNEK_H
