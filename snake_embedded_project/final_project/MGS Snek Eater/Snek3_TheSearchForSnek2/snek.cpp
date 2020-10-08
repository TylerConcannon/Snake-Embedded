#include "ui_snek.h"
#include "snek.h"
#include <QPainter>
#include <QKeyEvent>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPainterPath>
#include <glob.h>
#include <QDir>
#include <QInputDialog>
#include <QStringList>
#include <QObject>
#include <string.h>
#include <string>
#include <semaphore.h>
#include "fullsnake.h"
#include "datacache.h"

Snek::Snek(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Snek)
{
    ui->setupUi(this);

    //Set game
    DataCache::game = this;

    //Size the label
    ui->gameScreen->setMinimumSize(480, 270);

    //MAKE NEW SNAKE OBJECT
    DataCache::snek = new FullSnake();

    //Seed randomness
    srand(time(NULL));

    DataCache::pause_timer = 0;

    //Draw the screen
    DataCache::imgScreen = new QImage(480, 270, QImage::Format_RGB888);
    ui->gameScreen->setPixmap(QPixmap::fromImage(*DataCache::imgScreen));

    //Update screen
    updateScreen();

    //Draw snek image
    QString endpath = "/Snek3_TheSearchForSnek2/mgs_snek_eater.png";
    QPixmap pixmap(QDir::currentPath() + endpath);
    ui->gameScreen->setPixmap(pixmap);
    ui->gameScreen->repaint();
}

Snek::~Snek()
{
    delete ui;
}

void Snek::lose() {

    //Draw lose screen
    QString endpath = "/Snek3_TheSearchForSnek2/game_over.png";
    QPixmap pixmap(QDir::currentPath() + endpath);
    ui->gameScreen->setPixmap(pixmap);
    ui->gameScreen->repaint();

    //Enable button
    ui->newGameButton->setEnabled(true);
}

//Update the screen given an image
void Snek::updateScreen() {

    //Update if not null
    if(DataCache::imgScreen != NULL) {
        //Update label
        ui->gameScreen->setPixmap(QPixmap::fromImage(*DataCache::imgScreen));
        ui->gameScreen->repaint();
    }
}

//Sets the score label
void Snek::setScore(int value) {
    QString strScore = "Rations: " + QString::number(value);
    ui->scoreLabel->setText(strScore);
}

//Move up, left, right, down
void Snek::moveUp() {

    pthread_mutex_lock(&DataCache::snek_lock);

    if(DataCache::snek->getDir() != DataCache::snek->DOWN && !DataCache::dir_lock && !paused)
    {
        DataCache::dir_lock = 1;

        DataCache::snek->setDir(DataCache::snek->UP);
    }

    pthread_mutex_unlock(&DataCache::snek_lock);
}

void Snek::moveLeft() {

    pthread_mutex_lock(&DataCache::snek_lock);

    if (DataCache::snek->getDir() != DataCache::snek->RIGHT && !DataCache::dir_lock && !paused)
    {
        DataCache::dir_lock = 1;

        DataCache::snek->setDir(DataCache::snek->LEFT);
    }

    pthread_mutex_unlock(&DataCache::snek_lock);

}

void Snek::moveRight() {

    pthread_mutex_lock(&DataCache::snek_lock);

    if (DataCache::snek->getDir() != DataCache::snek->LEFT && !DataCache::dir_lock && !paused)
    {
        DataCache::dir_lock = 1;

        DataCache::snek->setDir(DataCache::snek->RIGHT);
    }

    pthread_mutex_unlock(&DataCache::snek_lock);
}

void Snek::moveDown() {

    pthread_mutex_lock(&DataCache::snek_lock);

    if (DataCache::snek->getDir() != DataCache::snek->UP && !DataCache::dir_lock && !paused)
    {
        DataCache::dir_lock = 1;

        DataCache::snek->setDir(DataCache::snek->DOWN);
    }

    pthread_mutex_unlock(&DataCache::snek_lock);
}

//Creates a new game
void Snek::createNewGame() {

    pthread_mutex_unlock(&DataCache::snek_lock);

    //MAKE NEW SNAKE OBJECT
    DataCache::snek = new FullSnake();

    //GET RESULT FROM DIALOG & STORE INTO GAME_SPEED
    QInputDialog qDialog;
    QStringList items;
    items << QString("BABY");
    items << QString("NORMAL");
    items << QString("EXTREME");
    items << QString("EUROPEAN EXTREME");

    qDialog.setOptions(QInputDialog::UseListViewForComboBoxItems);
    qDialog.setComboBoxItems(items);
    qDialog.setWindowTitle("SELECT DIFFICULTY");

    QObject::connect(&qDialog, SIGNAL(textValueSelected(const QString &)), this, SLOT(computeDifficulty(const QString &)));

    qDialog.exec();

    //Disable button
    ui->newGameButton->setEnabled(false);

    //Randomize pellet location
    DataCache::pellet_x = rand()%48;
    DataCache::pellet_y = rand()%27;

    //Reset the game flag & score
    DataCache::score = 0;
    ui->scoreLabel->setText("Rations: 0");
    DataCache::game_over = 0;
    DataCache::dir_lock = 0;

    //Post 3 game tokens so threads can start running
    sem_post(&DataCache::game_start_tokens);
    sem_post(&DataCache::game_start_tokens);
    sem_post(&DataCache::game_start_tokens);
    sem_post(&DataCache::game_start_tokens);
}

//Display story
void Snek::displayIntroduction() {

    //DISPLAY IMAGE HERE
    QString endpath = "/Snek3_TheSearchForSnek2/intro.png";
    QPixmap pixmap(QDir::currentPath() + endpath);
    ui->gameScreen->setPixmap(pixmap);
    ui->gameScreen->repaint();

}

//Computers difficulty for the game
void Snek::computeDifficulty(const QString & result) {
    if(result.compare("BABY") == 0) {
        DataCache::game_speed = EASY_MODE;
    }
    else if(result.compare("NORMAL") == 0) {
        DataCache::game_speed = MED_MODE;
    }
    else if(result.compare("EXTREME") == 0) {
        DataCache::game_speed = HARD_MODE;
    }
    else if(result.compare("EUROPEAN EXTREME") == 0) {
        DataCache::game_speed = EE_MODE;
    }
}

//Toggles the pause feature (NOT IMPLEMENTED)
void Snek::togglePause() {

    if(paused) {
        pthread_mutex_unlock(&DataCache::snek_lock);
        paused = 0;
    }
    else {
        pthread_mutex_lock(&DataCache::snek_lock);
        paused = 1;
    }

}

bool Snek::event(QEvent* event) {

    if (event->type()==QEvent::KeyPress && !paused && !DataCache::dir_lock) {

        pthread_mutex_lock(&DataCache::snek_lock);
        DataCache::dir_lock = 1;

        QKeyEvent* key = static_cast<QKeyEvent*>(event);
        if(key->key() == Qt::Key_W && DataCache::snek->getDir() != DataCache::snek->DOWN)
        {
            DataCache::snek->setDir(DataCache::snek->UP);
        }
        else if (key->key() == Qt::Key_S && DataCache::snek->getDir() != DataCache::snek->UP)
        {
            DataCache::snek->setDir(DataCache::snek->DOWN);
        }
        else if (key->key() == Qt::Key_D && DataCache::snek->getDir() != DataCache::snek->LEFT)
        {
            DataCache::snek->setDir(DataCache::snek->RIGHT);
        }
        else if (key->key() == Qt::Key_A && DataCache::snek->getDir() != DataCache::snek->RIGHT)
        {
            DataCache::snek->setDir(DataCache::snek->LEFT);
        }

        pthread_mutex_unlock(&DataCache::snek_lock);

    } else {
        return QWidget::event(event);
    }
    return false;

}
