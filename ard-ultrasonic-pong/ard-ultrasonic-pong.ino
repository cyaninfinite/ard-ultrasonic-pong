/*

  Arduino Ultrasonic Pong
  >> The classic pong game that uses an ultrasonic sensor to move the paddle.
     Powered by Arduino & u8g2 lib.

  By: 1487Quantum (https://github.com/1487quantum)

*/
#include <U8g2lib.h>

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
//Active range,cm (min,max)
uint8_t activeDist[2] = {10,40};

//Ball
uint8_t bw = 4;   //Width & height same

//Ball coordinate
uint8_t bx = WD / 2;
uint8_t by = HG / 2;

//Ball Speed
uint8_t bsx = 3;
uint8_t bsy = 3;

//Arena: Define paddle loc
uint8_t areaw = WD * 4 / 5; //Take up 4/5 of screen

//Paddle global
//Size
uint8_t pw = bw;  //Paddle width = ball width
uint8_t ph = 5 * bw;  //Paddle 5x bigger than ball

//Paddle 1
//Loc
uint8_t p1x = (WD / 2) - areaw / 2; //Paddle 1 x loc, fixed
uint8_t p1y = 10;                   //Paddle 1 y loc,centre
//Spd
uint8_t p1sy = 3;                   //Move up/down

//Paddle 2
//Loc
uint8_t p2x = (WD / 2) + areaw / 2; //Paddle 2 x loc, fixed
uint8_t p2y = 10;                   //Paddle 2 y loc,centre
//Spd
uint8_t p2sy = 2;                   //Move up/down

//Scoring
uint8_t p1Score = 0;          //Player 1 Score
uint8_t p2Score = 0;          //Player 1 Score
uint8_t winningScore = 11;    //Score to reach to win, keep it max at 99 (else text offset needs to be changed)

//Activation range for controls
bool activeRange() {
  return  dist >= activeDist[0] && dist <= activeDist[1] ;
}

void movPaddle() {
  //Control paddle 1 via distance
  if (activeRange) {
    p1y = map(dist, activeDist[0], activeDist[1], 0, HG);   //Remap distance to screen height (max)
  } else {
    if (by < p1y) {
      p1y -= p1sy;
    } else {
      p1y += p1sy;
    }
  }


  if (by < p2y) {
    p2y -= p2sy;
  } else {
    p2y += p2sy;
  }
  //Paddle collision
  if (p1y + ph > HG) {
    p1y = HG - ph;
  } else if (p1y < 0) {
    p1y = 0;
  }

  if (p2y + ph > HG) {
    p2y = HG - ph;
  } else if (p1y < 0) {
    p2y = 0;
  }

}

//Collision
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
    return bx <= p1x + pw && withinY(p1y);
  } else {
    return bx + bw >= p2x && withinY(p2y);
  }
}

//Check for paddle top/down collision, isFirst true: Check collision for first paddle (Left), if top true: check top
bool tpCollision(bool isFirst, bool top) {
  if (isFirst) {
    return top ? (by + bw >= p1y) : (by <= p1y + ph) && withinX(p1x);
  } else {
    return top ? (by + bw >= p2y) : (by <= p2y + ph) && withinX(p2x);
  }
}

void rdmSpd() {
  bsx = random(1, 5);
  bsy = random(1, 5);
}

void updatePos() {
  //Left,right collision
  if (bx - bw - 2 <= 0  || bx + bw / 2 >= WD)  {
    if (bx - bw - 2 <= 0) {
      p2Score++;
      rdmSpd();
      if (p2Score >= winningScore) {
        p2Score = 0;
      }
    } else {
      p1Score++;
      rdmSpd();
      if (p1Score >= winningScore) {
        p1Score = 0;
      }
    }
    bx = WD / 2;
    by = HG / 2;
  } else if (fCollision(1) || fCollision(0)) {
    bsx = -bsx;
  }
  if (by - bw / 2 <= 0 || by + bw / 2 >= HG ) {
    // Top,Bottom edge collision
    bsy = -bsy;
  }

  bx += bsx;
  by += bsy;

  movPaddle();
}

void drawBall() {
  updatePos();
  u8g2.drawBox(bx, by, bw, bw);
}

void drawPaddle() {
  u8g2.drawBox(p1x, p1y, pw, ph);
  u8g2.drawBox(p2x, p2y, pw, ph);
}

void drawText() {
  char sc1[3];  //P1
  char sc2[3];  //P2
  char dst[8];  //Dst
  sprintf (sc1, "%02d", p1Score);   //Displays 2 digits, shows 0 when score is below 10
  sprintf (sc2, "%02d", p2Score);
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
  delay(3000);    //Show for 2s
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
}

void loop() {
  u8g2.clearBuffer();
  drawText();
  drawBall();
  getDist();
  drawPaddle();
  u8g2.sendBuffer();
}
