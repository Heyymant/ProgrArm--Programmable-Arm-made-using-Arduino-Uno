#include<Servo.h>
#include<Keypad.h>
#include<EEPROM.h>

volatile char mode = 'L';  //L = lobby state
//byte noMovements = 5;  //noOfMovementsInAnInstruction  //remove this var at the end
volatile byte saveButton = 0;
volatile bool reset = true;
Servo servo1;  //floor
Servo servo2;  //hinge
Servo servo3;  //claw
int labelStore;
int labelExec;

volatile unsigned long int previousTime = 0;  //for save button
volatile unsigned long int currentTime;

//FOR INSTRUCTIONS

byte instruction[3][5];  //stores instruction

//FOR KEYPAD

const byte noRows = 4; 
const byte noCols = 4; 

char keyChar[noRows][noCols] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

byte rowPins[noRows] = {12, 8, 7, 6}; //{0, 1, 4, 5};
byte colPins[noCols] = {5, 4, 1, 0}; //{6, 7, 8, 12};

Keypad keypadIn(makeKeymap(keyChar), rowPins, colPins, noRows, noCols); 

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(2), saveButtonIncrementISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(3), resetPressedISR, FALLING);
  servo1.attach(9);  //floor
  servo2.attach(10);  //hinge
  servo3.attach(11);  //claw
}

void loop() {
  // put your main code here, to run repeatedly:    
  switch(mode){
    case 'L':   //wait for keypad mode entry
      if(reset){
        initialPosition();
        reset = false;
      }
      while(true){
        mode = keypadIn.getKey();  //get keypad input for mode
        if(mode != NO_KEY){break;}
      }
      if(mode=='A' || mode=='B' || mode=='C'){  //mode= 'D'  when mode D gets implemented
      }
      else{
         mode = 'L';
      }
      break;
    case 'A':   //program and execute
      //Serial.println('A');
      program();   //program the arm      
      execute();    //execte the programmed instruction          
      break;
    case 'B':   //store instructions
      char keyB;
      while(!reset){
        //labelStore = (byte)atoi(keypadIn.getKey());
        keyB = keypadIn.getKey();
        //labelStore =  key - '0';
        if(keyB != NO_KEY){break;}
      }
      if(keyB=='1' || keyB=='2'){     
        labelStore = keyB - '0'; 
        program();
        storeToEEPROM();
      }
      break;
    case 'C':   //retrieve stored instructions and execute
      char keyC;
      while(!reset){
          //labelExec = (byte)atoi(keypadIn.getKey());
          keyC = keypadIn.getKey();
          if(keyC != NO_KEY){break;}
        }
      if(keyC=='1' || keyC=='2'){
        labelExec = keyC - '0';
        retrieveFromEEPROM();
        execute();
      }
      break;

    //case 'D':   //gesture control
     // break;
  }
  
}

void initialPosition(){
    servo1.write(0);   //initial positions of all three servos, not necessarily 0
    servo2.write(70);
    servo3.write(130);
}

void program(){
    saveButton = 0;
    while((saveButton < 5) && !reset){
      byte servoPos1 = map(analogRead(A0), 0, 1023, 0, 180);  //value from pot 1 mapped to floor dof
      byte servoPos2 = map(analogRead(A1), 0, 1023, 80, 140);  //value from pot 2 mapped to hinge dof
      byte servoPos3 = map(analogRead(A2), 0, 1023, 130, 180);  //value from pot 3 mapped to claw dof
      servoPos1=constrain(servoPos1, 0, 180);
      servoPos2=constrain(servoPos2, 80, 140);
      servoPos3=constrain(servoPos3, 130, 180);
      instruction[0][saveButton] = servoPos1;
      instruction[1][saveButton] = servoPos2;
      instruction[2][saveButton] = servoPos3;
      servo1.write(servoPos1);
      servo2.write(servoPos2);
      servo3.write(servoPos3);
    }  
}

void execute(){   
  while(!reset){
   for(int j = 0; j < 5; j++){
      byte servoPos1=instruction[0][j];
      byte servoPos2=instruction[1][j];
      byte servoPos3=instruction[2][j];
      servoPos1=constrain(servoPos1, 0, 180);
      servoPos2=constrain(servoPos2, 80, 140);
      servoPos3=constrain(servoPos3, 130, 180);
      servo1.write(servoPos1);
      servo2.write(servoPos2);
      servo3.write(servoPos3);
      delay(1000);   //wait for some time after each movement
    }
   initialPosition();
  }
}

void storeToEEPROM(){
  if(!reset){
    int startIndex = 3*5*(labelStore-1);
    for(int i=0; i<3; i++){
      for(int j=0; j<5; j++){
        EEPROM.write(startIndex, instruction[i][j]);
        startIndex++;
      }
    }
  }
}

void retrieveFromEEPROM(){
  if(!reset){
    int startIndex = 3*5*(labelExec-1);
    for(int i=0; i<3; i++){
      for(int j=0; j<5; j++){
        instruction[i][j] = EEPROM.read(startIndex);
        startIndex++;
      }
    }
  }
}

void resetPressedISR(){
  reset = true;
  mode = 'L';
}

void saveButtonIncrementISR(){
  currentTime = millis();
  if((currentTime-previousTime)>20){
    saveButton++;
    previousTime = currentTime;
  }
}
