/*

  Arduino Ultrasonic Pong
  >> The classic pong game that uses an ultrasonic sensor to move the paddle.
     Powered by Arduino & u8g2 lib.

  By: 1487Quantum (https://github.com/1487quantum)

*/
#include <U8g2lib.h>
#include "player.h"

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//Pins
#define USPIN 9         //Ultrasound

//Display
#define WD 128
#define HG 64

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//Ultrasound
long duration, dist;
//Active range,cm
uint8_t distMin = 10;
uint8_t distMax = 40;

//Ball
uint8_t bw = 4;   //Width & height same

//Ball coordinate
uint8_t bx = WD / 2;
uint8_t by = HG / 2;

//Ball Speed
uint8_t bsx = 3;
uint8_t bsy = 3;

//Game frame: Define paddle loc
uint8_t areaw = WD * 4 / 5; //Take up 4/5 of screen

//Paddle global config
//Location
uint8_t pdy = 10;   //Y position of paddle would be fix at start
//Size
uint8_t pw = bw;  //Paddle width = ball width
uint8_t ph = 5 * bw;  //Paddle 5x bigger than ball
//Paddle Local config
//P1 Speed
uint8_t p1sy = 3;                   //Paddle 1 Movement Speed
//P2 Speed
uint8_t p2sy = 2;                   //Paddle 2 Movement Speed

//Scoring
uint8_t winningScore = 11;    //Score to reach to win, keep it max at 99 (else text offset needs to be changed)

//Player
Player* p1;
Player* p2;



//Activation range for controls
bool activeRange() {
  return  dist >= distMin && dist <= distMax ;
}

void movPaddle() {
  uint8_t pspd = 0;    //Temp var to get paddle speed

  //Control paddle 1 via distance
  if (activeRange()) {
    //Ultrasonic control
    p1->setYPos((uint8_t) map(dist, distMin, distMax, 0, HG));   //Remap distance to screen height (max)
  } else {
    //Auto mode control
    pspd = p1->getSpd();
    if (by < p1->getYPos()) {
      pspd *= -1;               //Flip direction, paddle moves up as is above
    }
    p1->setYPos(p1->getYPos() + pspd);    //Paddle moves down
  }

  //Player 2 paddle control
  pspd = p2->getSpd();
  if (by < p2->getYPos()) {
    pspd *= -1;               //Flip direction, paddle moves up as is above
  }
  p2->setYPos(p2->getYPos() + pspd);    //Paddle moves down

  //Paddle collision with screen edge
  if (p1->getYPos() + ph > HG) {
    p1->setYPos(HG - ph);
  } else if (p1->getYPos() < 0) {
    p1->setYPos(0);
  }

  if (p2->getYPos() + ph > HG) {
    p2->setYPos(HG - ph);
  } else if (p1->getYPos() < 0) {
    p2->setYPos(0);
  }

}

//Ball Collision
//Width Limit:Check whether ball is touching paddle x aixs
bool withinX(uint8_t xpos) {
  return bx + bw >= xpos && bx <= xpos + pw;
}

//Height Limit:Check whether ball is touching paddle y aixs
bool withinY(uint8_t ypos) {
  return by + bw >= ypos && by <= ypos + ph;
}

//Check for paddle front collision, isFirst true: Check collision for first paddle (Left)
bool fCollision(bool isFirst) {
  if (isFirst) {
    return bx <= p1->getXPos() + pw && withinY(p1->getYPos());
  } else {
    return bx + bw >= p2->getXPos() && withinY(p2->getYPos());
  }
}

//Check for paddle top/down collision, isFirst true: Check collision for first paddle (Left), if top true: check top
bool tpCollision(bool isFirst, bool top) {
  if (isFirst) {
    return top ? (by + bw >= p1->getYPos()) : (by <= p1->getYPos() + ph) && withinX(p1->getXPos());
  } else {
    return top ? (by + bw >= p2->getYPos()) : (by <= p2->getYPos() + ph) && withinX(p2->getXPos());
  }
}

void rdmSpd() {
  bsx = random(1, 5);
  bsy = random(1, 5);
}

//p1win, 0:P1 win, 1:P2 Win
void GameOver(bool p1win) {
  char t[15];  //Countdown time
  u8g2.clearBuffer();
  beep(1);  //Loooong beep
  //Restart in 3s
  for (uint8_t i = 5; i > 0; i--) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_17_tr);
    sprintf (t, "Restarts in %ds", i);
    u8g2.drawStr((WD / 2) - 32, (HG / 2 ) - 5, p1win ? "P1 Wins!" : "P2 Wins!");
    //Coundown restart
    u8g2.setFont(u8g2_font_6x12_t_symbols);
    u8g2.drawStr(WD / 5, HG * 5 / 6 , t);
    u8g2.sendBuffer();
    delay(1000);    //Delay 1s
  }
}

//p1, 0: P1, 1:P2
void updateScore(bool firstply) {
  (firstply ? p1 : p2)->addScore();
  rdmSpd();   //Random speed at start
  if ((firstply ? p1 : p2)->getScore() >= winningScore) {
    //Show gameover screen
    GameOver(firstply);
    //Reset score if winning score reached
    p1->resetScore();
    p2->resetScore();

  }
  delay(500);  //Pause 0.5 second after player score 1 point
}

void updatePos() {
  //Left,right collision
  if (bx - bw - 2 <= 0  || bx + bw / 2 >= WD)  {
    if (bx - bw - 2 <= 0) {
      updateScore(0); //Add score for player 2
    } else {
      updateScore(1); //Add score for player 1
    }
    bx = WD / 2;
    by = HG / 2;
  } else if (fCollision(1) || fCollision(0)) {
    beep(0);
    bsx = -bsx;
  }
  if (by - bw / 2 <= 0 || by + bw / 2 >= HG ) {
    // Top,Bottom edge collision
    beep(0);
    bsy = -bsy;
  }

  //Move ball
  bx += bsx;
  by += bsy;

  movPaddle();
}

void drawBall() {
  updatePos();
  u8g2.drawBox(bx, by, bw, bw);
}

void drawPaddle() {
  u8g2.drawBox(p1->getXPos(), p1->getYPos(), pw, ph);
  u8g2.drawBox(p2->getXPos(), p2->getYPos(), pw, ph);
}

void drawText() {
  char sc1[3];  //P1
  char sc2[3];  //P2
  char dst[8];  //Dst
  sprintf (sc1, "%02d", p1->getScore());   //Displays 2 digits, shows 0 when score is below 10
  sprintf (sc2, "%02d", p2->getScore());
  sprintf (dst, "D:%dcm", dist);

  //Scores
  u8g2.setFont(u8g2_font_fub17_tr);
  u8g2.drawStr((WD * 1 / 3) - 14, (HG / 2 ) - 12, sc1);
  u8g2.drawStr((WD / 2) - 3, (HG / 2 ) - 12, ":");
  u8g2.drawStr((WD * 2 / 3) - 8, (HG / 2 ) - 12, sc2);

  u8g2.setFont(u8g2_font_6x12_t_symbols);
  u8g2.drawStr(WD / 3, HG * 5 / 6 , dst);

}

void getDist() {
  // establish variables for duration of the ping,
  // and the distance result in inches and centimeters:

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(USPIN, OUTPUT);
  digitalWrite(USPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(USPIN, HIGH);
  delayMicroseconds(5);
  digitalWrite(USPIN, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(USPIN, INPUT);
  duration = pulseIn(USPIN, HIGH);

  // convert the time into a distance
  dist = microsecondsToCentimeters(duration);
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

void splash() {
  u8g2.setFont(u8g2_font_t0_17_tr  );
  u8g2.drawStr((WD / 2) - 17, (HG / 2 ) + 5, "PONG");
  u8g2.sendBuffer();
  delay(3000);    //Show for 3s
}

void initPlayers() {
  //Player 1, Left
  p1 = new Player((WD / 2) - areaw / 2, pdy, p1sy);
  //Player 2, Right
  p2 = new Player((WD / 2) + areaw / 2, pdy, p2sy);
}

//lg, true: Long beep
void beep(bool lg) {
  uint8_t dur = 50;
  if (lg) {
    dur *= 4; //Lengthen beep duration by 4
  }
  // to calculate the note duration, take one second divided by the note type.
  //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.

  //Buzzer pin 8, NOTE_C4: 262Hz, G4:392, noteDurations: 1000/4, quarter note
  tone(8, 392, 1000 / 4);
  delay(dur);
  noTone(8);
}

void setup() {
  // put your setup code here, to run once:
  DDRD |= 0b11111110; // set digital  1,2- 7 to output
  DDRB = 0b00111111; // set digital  8-13 to output

  //For Maker Uno: Turn off all onboard LEDS
  PORTD &= 0;
  PORTB &= 0;

  u8g2.begin();
  u8g2.setFlipMode(0);      //To flip display
  splash();                 //Show splashscreen
  initPlayers();             //Init player class
  beep(0);
}

void loop() {
  u8g2.clearBuffer();
  drawText();
  drawBall();
  getDist();
  drawPaddle();
  u8g2.sendBuffer();
}
