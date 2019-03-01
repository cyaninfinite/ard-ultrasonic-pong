#include "player.h"
/*
  ===========
  Constructor
  ===========
  Location of paddle, loc[x,y]
  Paddle Speed, Spd
*/

Player::Player(uint8_t pdx,uint8_t pdy, uint8_t spd) {
  px = pdx;
  py = pdy;
  s = spd;
  score=0;
}

//Paddle stuff
//Get paddle X position
uint8_t Player::getXPos() {
  return px;
}

//Get paddle Y position
uint8_t Player::getYPos() {
  return py;
}

//Set paddle position
void Player::setYPos(uint8_t y) {
  //px = x;
  py = y;
}

//Get paddle speed
uint8_t Player::getSpd() {
  return s;
}

//Set paddle speed
void Player::setSpd(uint8_t sp) {
  s = sp;
}

//Scores
//Setter, Increment score by one
void Player::addScore() {
  score++;
}

void Player::resetScore() {
  score = 0;
}

//Getter, get player's score
uint8_t Player::getScore() {
  return score;
}
