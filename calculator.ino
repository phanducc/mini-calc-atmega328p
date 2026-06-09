#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'C','7','4','1'},
  {'0','8','5','2'},
  {'=','9','6','3'},
  {'/','*','-','+'}
};

byte rowPins[ROWS] = {7, 6, 5, 4}; 
byte colPins[COLS] = {8, 9, 10, 11}; 

Keypad myKeypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

float numbers[20];
char operators[20];
int numIndex = 0;
int opIndex = 0;

float currentVal = 0;
boolean isTyping = false;
boolean done = false;
boolean hasError = false;
boolean hasOvf = false;

boolean lastWasOp = false;
int cursorX = 0;
int digitCount = 0;

float history[5];
int histCount = 0;
int viewIndex = -1;
boolean viewingHistory = false;

void setup() {
  lcd.init();      
  lcd.backlight(); 
  lcd.setCursor(0,0);
  lcd.print("Team36 MINI Calc"); 
  delay(1000);
  lcd.clear();
}

void loop() {
  char key = myKeypad.getKey(); 
  
  if (key) {
    if (key == 'C') {
      resetFunc();
    }
    else if (key >= '0' && key <= '9') {
      if (done || viewingHistory) resetFunc(); 
      
      if (digitCount < 7) {
        currentVal = currentVal * 10 + (key - '0');
        isTyping = true;
        lastWasOp = false; 
        lcd.print(key); 
        cursorX++;
        digitCount++;
      }
    }
    else if (key == '+' || key == '-' || key == '*' || key == '/') {
      
      if (viewingHistory) {
        currentVal = history[viewIndex];
        isTyping = true; 
        lcd.clear();
        lcd.print(currentVal); 
        cursorX = String(currentVal).length();
        viewingHistory = false;
      }

      if (done) { 
        done = false;
        lcd.clear();
        lcd.print(currentVal); 
        cursorX = String(currentVal).length();
        
        pushNumber(currentVal); 
        currentVal = 0;
        digitCount = 0;
      }
      else if (isTyping) {
        pushNumber(currentVal); 
        isTyping = false;
        currentVal = 0;
        digitCount = 0;
      } 
      else if (lastWasOp) {
        if (opIndex > 0) opIndex--; 
        if (cursorX > 0) {
          cursorX--;
          lcd.setCursor(cursorX, 0);
        }
      }
      else if (numIndex == 0) {
        return; 
      }
      
      lcd.print(key); 
      cursorX++;
      lastWasOp = true; 
      
      while (opIndex > 0 && getPriority(key) <= getPriority(operators[opIndex - 1])) {
        calculateStep();
      }
      
      pushOp(key); 
    }
    else if (key == '=') {
      if (viewingHistory || (!isTyping && numIndex == 0 && opIndex == 0)) {
        if (histCount > 0) {
          viewingHistory = true;
          viewIndex++;
          if (viewIndex >= histCount) viewIndex = 0; 
          
          lcd.clear();
          lcd.print("Hist ["); lcd.print(viewIndex + 1); lcd.print("]");
          lcd.setCursor(0, 1);
          lcd.print(history[viewIndex]);
        }
        return; 
      }

      boolean didCalc = (isTyping || opIndex > 0); 

      if (isTyping) {
        pushNumber(currentVal); 
        isTyping = false;
        digitCount = 0;
      }
      
      while (opIndex > 0) {
        calculateStep();
      }
      
      lcd.setCursor(0, 1);
      if (hasError) {
        lcd.print("Error            ");
        hasError = false; currentVal = 0;
      }
      else if (hasOvf) {
        lcd.print("ovf              ");
        hasOvf = false; currentVal = 0;
      }
      else {
        lcd.print("= ");
        if (numIndex > 0) {
          lcd.print(numbers[0]); 
          
          if (didCalc && !hasError && !hasOvf) {
            for (int i = 4; i > 0; i--) {
              history[i] = history[i-1]; 
            }
            history[0] = numbers[0]; 
            if (histCount < 5) histCount++;
          }

          currentVal = numbers[0];
          numIndex = 0; 
        } else {
          lcd.print("Err");
        }
      }
      done = true;
      lastWasOp = false;
    }
  }
}

void pushNumber(float val) {
  if (numIndex < 20) {
    numbers[numIndex] = val;
    numIndex++;
  }
}

void pushOp(char op) {
  if (opIndex < 20) {
    operators[opIndex] = op;
    opIndex++;
  }
}

int getPriority(char op) {
  if (op == '*' || op == '/') return 2;
  if (op == '+' || op == '-') return 1;
  return 0;
}

void calculateStep() {
  if (opIndex == 0) return; 
  
  if (numIndex < 2) {
    hasError = true;
    opIndex--; 
    return; 
  }
  
  float b = numbers[numIndex - 1]; numIndex--; 
  float a = numbers[numIndex - 1]; 
  
  char op = operators[opIndex - 1]; opIndex--; 
  
  float res = 0;
  if (op == '+') res = a + b;
  else if (op == '-') res = a - b;
  else if (op == '*') res = a * b;
  else if (op == '/') {
    if (b == 0) {
      hasError = true; 
      res = 0;
    } else {
      res = a / b;
    }
  }
  
  if (res > 16777215.0 || res < -16777215.0) {
    hasOvf = true;
    res = 0;
  }
  
  numbers[numIndex - 1] = res; 
}

void resetFunc() {
  lcd.clear();
  currentVal = 0;
  numIndex = 0;
  opIndex = 0;
  isTyping = false;
  done = false;
  hasError = false;
  hasOvf = false;
  lastWasOp = false;
  cursorX = 0;
  digitCount = 0;
  viewingHistory = false;
  viewIndex = -1;
}
