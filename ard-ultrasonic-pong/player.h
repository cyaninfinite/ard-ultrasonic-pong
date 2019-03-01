#include <stdint.h>

class Player
{
  private:
    //Player Var
    uint8_t score;            //Player Score
    //Paddle Var
    uint8_t px;               //Paddle x location,fixed
    uint8_t py;               //Paddle y location,centre
    uint8_t s;                //Paddle movement speed

  public:
    Player(uint8_t pdx, uint8_t pdy, uint8_t spd);   //Constructor
    //Paddle stuff
    uint8_t getXPos();             //Get Paddle X Position
    uint8_t getYPos();             //Get Paddle Y Position
    void setYPos(uint8_t y);                 //Set paddle Y position
    uint8_t getSpd();                 //Set paddle speed
    void setSpd(uint8_t sp);                 //Set paddle speed


    //Scores
    void addScore();               //Setter, Increment score by one
    void resetScore();               //Reset score
    uint8_t getScore();               //Getter, get player's score
};
