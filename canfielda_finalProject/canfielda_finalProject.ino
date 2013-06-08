#include <BallyLib.h>

#define DEBUG            1

#define N_BALLS          3
#define MAX_PLAYERS      4
#define MAX_CREDITS     99

#define REPLAY      100000

#define SWITCH_ROWS      3

#define BUTTON_UP        1
#define BUTTON_DOWN      0
#define TRUE             1
#define FALSE            0

#define COL_0     (1 <<  0)
#define COL_1     (1 <<  1)
#define COL_2     (1 <<  2)
#define COL_3     (1 <<  3)
#define COL_4     (1 <<  4)
#define COL_5     (1 <<  5)
#define COL_6     (1 <<  6)
#define COL_7     (1 <<  7)
#define COL_8     (1 <<  8)
#define COL_9     (1 <<  9)
#define COL_10    (1 << 10)
#define COL_11    (1 << 11)
#define COL_12    (1 << 12)
#define COL_13    (1 << 13)
#define COL_14    (1 << 14)
#define COL_15    (1 << 15)

Bally bally;

struct GameState {
  int cabinet;
  int firstTime;
  int firstHit;
  int inplay;
  int over;
  unsigned long highscore;
};

GameState game;

int credits;
int playerCount;
int curPlayer;
int curBall;
int highScore;
unsigned long score[MAX_PLAYERS];

int laneState;
int dropTargetState;

int bonusBumpers;

unsigned char switchState;



void setup() {
  credits = 0;
  
  clearAllLamps();
  clearAllDisplays();
  
  game.highscore = 200000;
  
  Serial.begin(9600);
  Serial.println("\n\nsetup complete...");
}

void loop() {
  int i;
  int j;  
  
  static int prevCreditState = BUTTON_UP;
  static int prevCoinState   = BUTTON_DOWN;
  
  playerCount    = -1;
  curPlayer      = -1;
  curBall        =  1;
  
  game.cabinet   = TRUE;
  game.firstTime = TRUE;
  game.firstHit  = TRUE;
  game.inplay    = FALSE;
  game.over      = FALSE;
  
  laneState        = 0;
  dropTargetState  = 0;

  bonusBumpers    = FALSE;
  
  // enable lamp: game over 
  bally.setLamp(12, 2, TRUE);
  
  // disable lamp: ball in play
  bally.setLamp(12, 0, FALSE);
  
  // disable lamps: 1-4 can play
  bally.setLampRow(13, FALSE);
  
  // disable flippers
  bally.setContSolenoid(2, TRUE);
  
  // clear balls
  clearDisplay(4);

  setCredits();
  
  while (!game.over) {
    
    if (game.firstHit && bally.getNextEnterTest() == TEST) {
      Serial.print("test button down...");
      credits = 0;
      setCredits();
      game.over = TRUE;
    }
    
    // coin button
    if (game.cabinet && bally.getCabSwitch(1, 0)) {
      if (prevCoinState == BUTTON_UP) {
        Serial.println("coin button down...");
        prevCoinState = BUTTON_DOWN;
        
        if (credits < MAX_CREDITS) {
          credits++;
          setCredits();
        }
      }  
    } else {
      prevCoinState = BUTTON_UP;
    } // end coin button


    // credit button
    if (game.cabinet && bally.getCabSwitch(0, 5)) {
      if (prevCreditState == BUTTON_UP) {
        Serial.println("credit button down...");
        prevCreditState = BUTTON_DOWN;
        
        if (credits > 0 && playerCount < (MAX_PLAYERS - 1)) {
          if (game.firstTime) {
            Serial.println("first time....");
            curPlayer = 0;
            game.firstTime = FALSE;
            game.inplay = TRUE;
            
            for (i = 0; i < MAX_PLAYERS; i++)
              score[i] = 0;
            clearAllDisplays();
            
            setBall();
                        
            // disable game over light
            bally.setLamp(12, 2, FALSE);
            
            // enable flippers
            bally.setContSolenoid(2, FALSE);
      
            // eject ball
            bally.fireSolenoid(6, TRUE); 
          }
          
          credits--;
          playerCount++;
          
          setCredits();
          resetTable();
          
          setDisplay(playerCount, 0);
        }
      }
    } else {
      prevCreditState = BUTTON_UP;
    } // end credit button


    switchState = bally.getDebRedgeRow(2);
    // right drop target d
    if (game.inplay && switchState & COL_0) {
      Serial.println("right drop target d");
      dropTargetState = (dropTargetState | COL_0);
      addScore(500);
      incrementBonus();
    }

    // right drop target c
    if (game.inplay && switchState & COL_1) {
      Serial.println("right drop target c");
      dropTargetState = (dropTargetState | COL_1);
      addScore(500);
      incrementBonus();
    }
    
    // right drop target b
    if (game.inplay && switchState & COL_2) {
      Serial.println("right drop target b");
      dropTargetState = (dropTargetState | COL_2);
      addScore(500);
      incrementBonus();
    }
        
    // right drop target a
    if (game.inplay && switchState & COL_3) {
      Serial.println("right drop target a");
      dropTargetState = (dropTargetState | COL_3);
      addScore(500);
      incrementBonus();
    }
    
    // left drop target d
    if (game.inplay && switchState & COL_4) {
      Serial.println("left drop target d");
      dropTargetState = (dropTargetState | COL_4);
      addScore(500);
      incrementBonus();
    }

    // left drop target c
    if (game.inplay && switchState & COL_5) {
      Serial.println("left drop target c");
      dropTargetState = (dropTargetState | COL_5);
      addScore(500);
      incrementBonus();
    }
    
    // left drop target b
    if (game.inplay && switchState & COL_6) {
      Serial.println("left drop target b");
      dropTargetState = (dropTargetState | COL_6);
      addScore(500);
      incrementBonus();
    }
    
    
    // left drop target a
    if (game.inplay && switchState & COL_7) {
      Serial.println("left drop target a");
      dropTargetState = (dropTargetState | COL_7);
      addScore(500);
      incrementBonus();
    }
    
    // reset drop targets
    if (game.inplay && dropTargetState == 255) {
      Serial.println("drop target bonus");
      bally.fireSolenoid(7, TRUE, TRUE);
      bally.fireSolenoid(3, TRUE, TRUE);
      dropTargetState = 0;
      addScore(50000);
    }
    
    
    switchState = bally.getRedgeRow(3);
    // right flipper feed lane
    if (game.inplay && switchState & COL_0) {
      Serial.println("right flipper feed lane");
      addScore(500);
      bally.setLamp(7, 2, TRUE);
      incrementBonus();
    }

    // left flipper feed lane
    if (game.inplay && switchState & COL_1) {
      Serial.println("left flipper feed lane");
      addScore(500);
      bally.setLamp(7, 3, TRUE);
      incrementBonus();
    }
    
    // drop target rebound
    if (game.inplay && switchState & COL_2) {
      Serial.println("drop target rebound");
      addScore(100);
    }
        
    // right b lane
    if (game.inplay && switchState & COL_3) {
      Serial.println("right b lane");
      bally.setLamp(3, 2, TRUE);
      laneState = laneState | COL_1;
    }
    
    
    // left a lane
    if (game.inplay && switchState & COL_4) {
      Serial.println("left a lane");
      bally.setLamp(3, 3, TRUE);
      laneState = laneState | COL_0;      
    }

    // top b lane
    if (game.inplay && switchState & COL_5) {
      Serial.println("top b lane");
      bally.setLamp(3, 2, TRUE);
      laneState = laneState | COL_1;
    }
    
    // top a lane
    if (game.inplay && switchState & COL_6) {
      Serial.println("top a lane");
      bally.setLamp(3, 3, TRUE);
      laneState = laneState | COL_0;
    }
    
    if (laneState == 3) {
      Serial.println("increment lane state");
      incrementLaneValue();
      bally.setLampRow(3, FALSE);
      laneState = 0;  
    }
    
    // top center kick out / saucer
    if (game.inplay && switchState & COL_7) {
      Serial.println("saucer");
      bally.fireSolenoid(0, TRUE);
      incrementMultiplier();
      addScore(3000);
    }

        
    switchState = bally.getSwitchRow(4);
    // right slingshot
    if (game.inplay && switchState & COL_2) {
      Serial.println("right slingshot");
      bally.fireSolenoid(11, TRUE);
      addScore(50);
    }
    
    // right out lane
    if (game.inplay && bally.getRedge(4, 0)) {
      Serial.println("right out lane");
      if (bally.getLampRow(7) & COL_3)
        addScore(50000);
      else 
        addScore(1000);
    }

    // left out lane
    if (game.inplay && bally.getRedge(4, 1)) {
      Serial.println("left out lane");
      if (bally.getLampRow(7) & COL_2)
        addScore(50000);
      else
        addScore(1000);
    }

    
    
    // left slingshot
    if (game.inplay && switchState & COL_3) {
      Serial.println("left slingshot");
      bally.fireSolenoid(13, TRUE);
      addScore(100);
    }
    
    // bottom right pop/thumper bumper
    if (game.inplay && switchState & COL_4 && bonusBumpers) {
        Serial.println("bottom right pop bumper"); 
        bally.fireSolenoid(5, TRUE);
        bally.fireSolenoid(2, TRUE, TRUE);
        addScore(1000);
    }
    
    // bottom left pop/thumper bumper
    if (game.inplay && switchState & COL_5 && bonusBumpers) { 
        Serial.println("bottom left pop bumper");
        bally.fireSolenoid(14, TRUE);
        bally.fireSolenoid(2, TRUE, TRUE);
        addScore(100);
    }
    
    // top right pop/thumper bumper
    if (game.inplay && switchState & COL_6) {
        Serial.println("top right pop bumper"); 
        bally.fireSolenoid(9, TRUE);
        bally.fireSolenoid(2, TRUE, TRUE);
        addScore(100);
    }
    
    // top left pop/thumper bumper
    if (game.inplay && switchState & COL_7) {
        Serial.println("top left pop bumper"); 
        bally.fireSolenoid(1, TRUE);
        bally.fireSolenoid(2, TRUE, TRUE);
        addScore(100);
    }
      
    // outhole
    if (game.inplay && bally.getDebounced(0, 7)) {
      Serial.println("outhole");
      if (bally.getLampRow(10) & COL_2 || bally.getLampRow(6) & COL_1) {
        // shoot again if player didn't make any points or if player has an extra ball
        
        Serial.println("shoot again!");
        // clear free ball
        bally.setLamp(6, 1, FALSE);
        
        
      } else {
        // bonus
        addScore(getBonus() * getMultiplier());
        laneState        = 0;
        dropTargetState  = 0;
        bonusBumpers    = FALSE;

        // reset drop targets
        bally.fireSolenoid(7, TRUE, TRUE);
        bally.fireSolenoid(3, TRUE, TRUE);
        
        if (++curPlayer > playerCount) {
          curPlayer = 0;
          
          
                  
          if (++curBall < (N_BALLS + 1)) {
            setBall();
            game.firstHit = TRUE;
          } else {
            game.over = TRUE;
          }
        }
        
        clearAllLamps();
        resetTable();        


      }
      
        
      if (!game.over) {
        // eject the ball 
        bally.fireSolenoid(6, TRUE);
        delay(1000);
      }
    } // end outhole
    
    

  } // end while
  
  
  // high score and free games
  for (i = 0; i < playerCount; i++) {
    if (score[i] > 200000) {
      credits++;
      bally.fireSolenoid(12, TRUE);
    }
    
    if (score[i] > 340000) {
      credits++;
      bally.fireSolenoid(12, TRUE);
    }
    
    if (score[i] > 480000) {
      credits++;
      bally.fireSolenoid(12, TRUE);
    }
    
    if (score[i] > game.highscore) {
      game.highscore = score[i];
      bally.setLamp(12, 1, TRUE);
    }
  }

  
  // Match and knocker
  if (random(0, 10) == 5) {
    bally.fireSolenoid(10, TRUE);
    bally.setLamp(10, 1, TRUE); 
  }
  
}

void addScore(unsigned long points) {
    game.inplay   = TRUE;
    game.cabinet  = FALSE;
    game.firstHit = FALSE;
    
    // disable lamp: shoot again
    bally.setLamp(10, 0, FALSE);
    // disable lamp: shoot again
    bally.setLamp(10, 2, FALSE);

  
  
  score[curPlayer] += points;
  setDisplay(curPlayer, score[curPlayer]);

  Serial.print("player: ");
  Serial.print(curPlayer);
  Serial.print(" points: ");
  Serial.print(points);
  Serial.print(" score: ");
  Serial.println(score[curPlayer]);
}

int getBonus() {
  int bonus;
  unsigned char lamps[3];
  
  bonus = 0;
  
  lamps[0] = bally.getLampRow(0);
  lamps[1] = bally.getLampRow(1);
  lamps[2] = bally.getLampRow(2);

  if (lamps[0] & COL_0)
    bonus += 1000;
  if (lamps[0] & COL_1)
    bonus += 2000;
  if (lamps[0] & COL_2)
    bonus += 3000;
  if (lamps[0] & COL_3)
    bonus += 4000;
  if (lamps[1] & COL_0)
    bonus += 5000;
  if (lamps[1] & COL_1)
    bonus += 6000;
  if (lamps[1] & COL_2)
    bonus += 7000;
  if (lamps[1] & COL_3)
    bonus += 8000;
  if (lamps[2] & COL_0)
    bonus += 9000;
  if (lamps[2] & COL_1)
    bonus += 10000;
  if (lamps[2] & COL_2)
    bonus += 20000;

  return bonus;
}

void incrementBonus() {
  int bonus;
  
  // chimes the bell
  bally.fireSolenoid(4, TRUE);
  
  // get bonus BEFORE clearing lights
  bonus = getBonus();

  if (bonus < 29000)
    bonus += 1000;

  // clear bonus lamps
  bally.setLampRow(0, FALSE);
  bally.setLampRow(1, FALSE);
  bally.setLampRow(2, FALSE);
  
  // set bonus lamps
  if (bonus >= 10000 && bonus < 20000) {
    bally.setLamp(2, 1, TRUE);
    bonus -= 10000;
  }
  else if (bonus >= 20000) {
    bally.setLamp(2, 2, TRUE);
    bonus -= 20000;
  }
  
  switch (bonus) {
    case 1000:
      bally.setLamp(0, 0, TRUE);  
    break;
    case 2000:
      bally.setLamp(0, 1, TRUE);  
    break;    
    case 3000:
      bally.setLamp(0, 2, TRUE);  
    break;    
    case 4000:
      bally.setLamp(0, 3, TRUE);  
    break;    
    case 5000:
      bally.setLamp(1, 0, TRUE);  
    break;    
    case 6000:
      bally.setLamp(1, 1, TRUE);  
    break;    
    case 7000:
      bally.setLamp(1, 2, TRUE);  
    break;    
    case 8000:
      bally.setLamp(1, 3, TRUE);
    break;
    case 9000:
      bally.setLamp(2, 0, TRUE);
    break;
  } 
  
}


int getMultiplier() {
  unsigned char lamps;
  
  lamps = bally.getLampRow(9);
  
  if (lamps == 0)
    return 0;
  if (lamps & COL_3)
    return 2;
  if (lamps & COL_2)
    return 3;
  if (lamps & COL_1)
    return 5;
}

void incrementMultiplier() {
  
  // chimes the bell
  bally.fireSolenoid(8, TRUE);
  
  switch (getMultiplier()) {
    case 0:
      bally.setLamp(9, 3, TRUE);
      bally.setLamp(11, 3, TRUE);
    break;
    case 2:
      bally.setLamp(9, 3, FALSE);
      bally.setLamp(9, 2, TRUE);
      bally.setLamp(11, 3, FALSE);
      bally.setLamp(11, 2, TRUE);
    break;
    case 3:
      bally.setLamp(9, 2, FALSE);
      bally.setLamp(9, 1, TRUE);
      bally.setLamp(11, 2, FALSE);
      bally.setLamp(11, 1, TRUE);
    break;   
  }
}

int getLaneValue() {
  unsigned char lamps[2];
  
  lamps[0] = bally.getLampRow(5);
  lamps[1] = bally.getLampRow(6);

  if (lamps[0] & COL_0)
    return 1000;
  if (lamps[0] & COL_1)
    return 2000;
  if (lamps[0] & COL_2)
    return 3000;
  if (lamps[0] & COL_3)
    return 4000;
  if (lamps[1] & COL_0)
    return 5000;
  if (lamps[1] & COL_1)
    return 6000;
  if (lamps[1] & COL_2)
    return 7000;
   
  return 0; 
}


void incrementLaneValue() {
  int laneValue;
  
  // chime the bell
  bally.fireSolenoid(12, TRUE);

  // enable bonus bumpers
  bonusBumpers = TRUE;
  bally.setLampRow(8, TRUE);
 
  // get lane value before resetting lamps
  laneValue = getLaneValue();
  if (laneValue < 7000)
    laneValue += 1000;
  
  Serial.print("lane value: ");
  Serial.println(laneValue);
    
  // clear lamps
  bally.setLampRow(5, FALSE);  
  bally.setLampRow(6, FALSE);
  
  switch(laneValue) {
    case 1000:
      bally.setLamp(5, 0, TRUE); 
    break;
    case 2000:
      bally.setLamp(5, 1, TRUE); 
    break;
    case 3000:
      bally.setLamp(5, 2, TRUE); 
    break;
    case 4000:
      bally.setLamp(5, 3, TRUE); 
    break;
    case 5000:
      bally.setLamp(6, 0, TRUE);
    break;
    case 6000:
      bally.setLamp(6, 0, TRUE);
      bally.setLamp(6, 1, TRUE);
    break;
    case 7000:
      bally.setLamp(6, 1, TRUE);
      bally.setLamp(6, 2, TRUE);
    break;
  }
  
  addScore(laneValue);
}

void resetTable() {
  int i;
  
  Serial.println("resetTable()");
  
  // saucer
  // bally.fireSolenoid(0, TRUE); 
  
  // top left pop bumer
  // bally.fireSolenoid(1, TRUE); 
  
  // chime 10000
  // bally.fireSolenoid(2, TRUE); 
  
  // left bank
  bally.fireSolenoid(3, TRUE, TRUE); 
  
  // chime 100
  // bally.fireSolenoid(4, TRUE); 
  
  // bottom right pop bumer
  // bally.fireSolenoid(5, TRUE); 
  
  // ball eject
  // bally.fireSolenoid(6, TRUE); 
  
  // right bank
  bally.fireSolenoid(7, TRUE, TRUE); 
  
  // chime 10
  // bally.fireSolenoid(8, TRUE); 
  
  // top right pop bumper
  // bally.fireSolenoid(9, TRUE); 
  
  // knocker
  // bally.fireSolenoid(10, TRUE);  
  
  // right slingshot
  // bally.fireSolenoid(11, TRUE); 
  
  // chime 1000
  // bally.fireSolenoid(12, TRUE); 
  
  // left slingshot
  // bally.fireSolenoid(13, TRUE); 
  
  
  // disable lamps: 1-4 can play
  bally.setLampRow(13, FALSE);
  
  // enable lamps: 1-4 can play
  bally.setLamp(13, playerCount, TRUE);
  
  // disable lamps: player up
  bally.setLampRow(14, FALSE);

  Serial.println(curPlayer);  
  // enable lamps: player up
  bally.setLamp(14, curPlayer, TRUE);
            
  // enable light: ball in play
  bally.setLamp(12, 0, TRUE);
            
  // enable light: bonus 1k
  bally.setLamp(0, 0, TRUE);
            
  // enable light: lane 1k
  bally.setLamp(5, 0, TRUE);
  
  // enable light: shoot again
  bally.setLamp(10, 2, TRUE);
  
}

void setCredits() {
  setDisplay(4, credits, 3);
}

void setBall() {
  setDisplay(4, curBall, 0);
} 

void clearAllLamps() {
  for (int i = 0; i < N_LAMP_ROWS; i++)
    bally.setLampRow(i, 0);
}

void clearAllDisplays() {
  for (int i = 0; i < N_DISPLAYS; i++)
    clearDisplay(i); 
}

void clearDisplay(int player) {
  for (int i = 0; i < N_DIGITS; i++)
    bally.setDisplay(player, i, 10);
}

void clearDisplay(int player, int pos, int digits) {
  for (int i = 0; i < digits; i++)
    bally.setDisplay(player, pos + i, 10);  
}

void setDisplay(int player, long value) {
  setDisplay(player, value, 0);
}

void setDisplay(int player, long value, int pos) {
  int i;
  int digits = countDigits(value);

  clearDisplay(player, pos, digits);
    
  for (i = 0; i < digits; i++) {
    bally.setDisplay(player, pos + i, value % 10);
    value /= 10;
  }
}

int countDigits(long value) {
  int digits = 0;
  
  if (value == 0)
    return 1;
  
  while (value != 0) {
    value /= 10;
    digits++;
  }
  
  return digits;
}
