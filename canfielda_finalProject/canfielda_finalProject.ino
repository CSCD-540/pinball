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
};

GameState game;

int credits;
int playerCount;
int curPlayer;
int curBall;
int highScore;
int score[MAX_PLAYERS];

int leftBankState;
int rightBankState;

unsigned char switchState;



void setup() {
  credits = 0;
  
  clearAllLamps();
  clearAllDisplays();
  
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
  
  leftBankState  = 0;
  rightBankState = 0;
  
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
    Serial.println(playerCount);
    
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
            resetTable();
            
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
          
          // enable player light
          bally.setLamp(14, playerCount, TRUE);
          setDisplay(playerCount, 0);
        }
      }
    } else {
      prevCreditState = BUTTON_UP;
    } // end credit button
Serial.print("inplay: ");
Serial.println(game.inplay);

    switchState = bally.getDebRedgeRow(2);
    // right drop target d
    if (game.inplay && switchState & COL_0) {
      rightBankState = (rightBankState | COL_0);
      addScore(500);
    }

    // right drop target c
    if (game.inplay && switchState & COL_1) {
      rightBankState = (rightBankState | COL_1);
      addScore(500);
    }
    
    // right drop target b
    if (game.inplay && switchState & COL_2) {
      rightBankState = (rightBankState | COL_2);
      addScore(500);
    }
        
    // right drop target a
    if (game.inplay && switchState & COL_3) {
      rightBankState = (rightBankState | COL_3);
      addScore(500);
    }
    
    // left drop target d
    if (game.inplay && switchState & COL_4) {
      leftBankState = (leftBankState | COL_0);
      addScore(500);
    }

    // left drop target c
    if (game.inplay && switchState & COL_5) {
      leftBankState = (leftBankState | COL_1);
      addScore(500);
    }
    
    // left drop target b
    if (game.inplay && switchState & COL_6) {
      leftBankState = (leftBankState | COL_2);
      addScore(500);
    }
    
    
    // left drop target a
    if (game.inplay && switchState & COL_7) {
      leftBankState = (leftBankState | COL_3);
      addScore(500);
    }
    
    // reset right bank
    if (game.inplay && rightBankState == 15) {
      bally.fireSolenoid(7, TRUE, TRUE);
      rightBankState = 0;
      addScore(1000);
    }
    
    // reset left bank
    if (game.inplay && leftBankState == 15) {
      bally.fireSolenoid(3, TRUE, TRUE);
      leftBankState = 0;
      addScore(1000);
    }
    
    
    switchState = bally.getDebouncedRow(3);
    // right flipper feed lane
    if (game.inplay && switchState & COL_0) {
      
      addScore(100);
    }

    // left flipper feed lane
    if (game.inplay && switchState & COL_1) {
      
      addScore(100);
    }
    
    // drop target rebound
    if (game.inplay && switchState & COL_2) {

      addScore(100);
    }
        
    // right b lane
    if (game.inplay && switchState & COL_3) {

      addScore(100);
    }
    
    
    // left a lane
    if (game.inplay && switchState & COL_4) {
      
      addScore(100);
    }

    // top b lane
    if (game.inplay && switchState & COL_5) {
      
      addScore(100);
    }
    
    // top a lane
    if (game.inplay && switchState & COL_6) {

      addScore(100);
    }
    
    // top center kick out / saucer
    if (game.inplay && switchState & COL_7) {
      bally.fireSolenoid(0, TRUE);
      incrementMultiplier();
      addScore(3000);
    }

        
    switchState = bally.getSwitchRow(4);
    // right slingshot
    if (game.inplay && switchState & COL_2) {
      bally.fireSolenoid(11, TRUE);
      addScore(100);
    }
    
    // right out lane
    if (game.inplay && bally.getRedge(4, 0)) {
      
      addScore(100);
    }

    // left out lane
    if (game.inplay && bally.getRedge(4, 1)) {
      
      addScore(100);
    }

    
    
    // left slingshot
    if (game.inplay && switchState & COL_3) {
      bally.fireSolenoid(13, TRUE);
      addScore(100);
    }
    
    // bottom right pop/thumper bumper
    if (game.inplay && switchState & COL_4) { 
        bally.fireSolenoid(5, TRUE);
        addScore(100);
    }
    
    // bottom left pop/thumper bumper
    if (game.inplay && switchState & COL_5) { 
        bally.fireSolenoid(14, TRUE);
        addScore(100);
    }
    
    // top right pop/thumper bumper
    if (game.inplay && switchState & COL_6) { 
        bally.fireSolenoid(9, TRUE);
        addScore(100);
    }
    
    // top left pop/thumper bumper
    if (game.inplay && switchState & COL_7) { 
        bally.fireSolenoid(1, TRUE);
        addScore(100);
    }

      Serial.print("inplay: ");
      Serial.println(game.inplay);
      
    // outhole
    if (game.inplay && bally.getDebounced(0, 7)) {
      
      if (bally.getLampRow(10) & COL_2 || bally.getLampRow(6) & COL_1) {
        ; // shoot again if player didn't make any points or if player has an extra ball
      } else {
        // TODO: calculate bonuses  
        if (++curPlayer > playerCount) {
          curPlayer = 0;
        
          if (++curBall < (N_BALLS + 1)) {
            setBall();
          } else {
            game.over = TRUE;
          }
        }

      }
      
        
      if (!game.over) {
        resetTable();
        
        game.firstHit = TRUE;
        // eject the ball 
        bally.fireSolenoid(6, TRUE);
        
      }
    } // end outhole
    
    

  } // end while
  
  

  
  // TODO: Match and knocker


  
}

void addScore(int points) {
  if (game.firstHit) {
    game.inplay   = TRUE;
    game.cabinet  = FALSE;
    game.firstHit = FALSE;
    
    // disable lamp: shoot again
    bally.setLamp(10, 0, FALSE);
    // disable lamp: shoot again
    bally.setLamp(10, 2, FALSE);
  }
  
  score[curPlayer] += points;
  setDisplay(curPlayer, score[curPlayer]);
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

void resetTable() {
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
          
  // enable lamps: current player can play
  bally.setLamp(13, curPlayer, TRUE);
            
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
