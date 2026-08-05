#include <SoftwareSerial.h>


/* --------- HC-05 Bluetooth --------- */
#define BT_RX A0   // Arduino RX  (HC-05 TX -> A0)
#define BT_TX A1   // Arduino TX  (HC-05 RX <- A1)
#define BT_BAUD 9600


SoftwareSerial bt(BT_RX, BT_TX);


/* --------- Ultrasonic --------- */
const uint8_t Trig = A3, Echo = A2;


/* --------- Motor / shift reg pins --------- */
const uint8_t PWM2A=11, PWM2B=3, PWM0A=6, PWM0B=5;
const uint8_t DIR_CLK=4, DIR_EN=7, DATA=8, DIR_LATCH=12;


/* --------- Direction codes --------- */
const uint8_t Move_Forward=39, Move_Backward=216, Left_Move=116, Right_Move=139;
const uint8_t Right_Rotate=149, Left_Rotate=106, Stop=0;
const uint8_t Upper_Left_Move=36, Upper_Right_Move=3, Lower_Left_Move=80, Lower_Right_Move=136;


/* --------- Speeds --------- */
int Speed1=200, Speed2=200, Speed3=200, Speed4=200;


/* --------- Modes --------- */
enum RunMode { MODE_MANUAL, MODE_FOLLOW, MODE_AVOID };
RunMode runMode = MODE_MANUAL;
bool Flag = true;


/* --------- Motor function --------- */
void Motor(uint8_t Dir,int s1,int s2,int s3,int s4){
  analogWrite(PWM2A,s1); analogWrite(PWM2B,s2);
  analogWrite(PWM0A,s3); analogWrite(PWM0B,s4);
  digitalWrite(DIR_LATCH,LOW);
  shiftOut(DATA,DIR_CLK,MSBFIRST,Dir);
  digitalWrite(DIR_LATCH,HIGH);
}


/* --------- Ultrasonic --------- */
int readDistance(){
  digitalWrite(Trig,LOW); delayMicroseconds(2);
  digitalWrite(Trig,HIGH); delayMicroseconds(10);
  digitalWrite(Trig,LOW);
  long duration = pulseIn(Echo,HIGH,30000);
  if(duration==0) return -1;
  return duration/58;
}


/* --------- FOLLOW --------- */
void Ultrasonic_Follow(){
  int d = readDistance();


  if(d<0){
    Motor(Stop,0,0,0,0);
    return;
  }


  if(d > 50){
    Motor(Move_Forward,150,150,150,150);
  }
  else if(d < 30){
    Motor(Move_Backward,150,150,150,150);
  }
  else{
    Motor(Stop,0,0,0,0);
  }
}


/* --------- AVOID --------- */
void Ultrasonic_Avoidance(){
  int d = readDistance();


  if(d>0 && d<40){
    Motor(Stop,0,0,0,0);
    delay(200);


    Motor(Move_Backward,150,150,150,150);
    delay(300);


    Motor(Flag?Left_Rotate:Right_Rotate,150,150,150,150);
    Flag=!Flag;
    delay(300);
  }
  else{
    Motor(Move_Forward,120,120,120,120);
  }
}


/* --------- Command --------- */
void doCmd(char cmd){
  Serial.print("CMD: ");
  Serial.println(cmd);


  switch(cmd){


    // JOYSTICK GAUCHE
    case 'c': Motor(Move_Forward,Speed1,Speed2,Speed3,Speed4); runMode=MODE_MANUAL; break;
    case 'd': Motor(Move_Backward,Speed1,Speed2,Speed3,Speed4); runMode=MODE_MANUAL; break;
    case 'a': Motor(Left_Move,Speed1,Speed2,Speed3,Speed4); runMode=MODE_MANUAL; break;
    case 'b': Motor(Right_Move,Speed1,Speed2,Speed3,Speed4); runMode=MODE_MANUAL; break;
    case 'r': Motor(Stop,0,0,0,0); runMode=MODE_MANUAL; break;


    // JOYSTICK DROIT
    case 'e': Motor(Left_Rotate,Speed1,Speed2,Speed3,Speed4); runMode=MODE_MANUAL; break;
    case 'f': Motor(Right_Rotate,Speed1,Speed2,Speed3,Speed4); runMode=MODE_MANUAL; break;
    case 'g': Motor(Move_Forward,Speed1,Speed2,Speed3,Speed4); runMode=MODE_MANUAL; break;
    case 'h': Motor(Move_Backward,Speed1,Speed2,Speed3,Speed4); runMode=MODE_MANUAL; break;
    case 'y': Motor(Stop,0,0,0,0); runMode=MODE_MANUAL; break;


    // MODES
    case 'm': runMode=MODE_FOLLOW; break;
    case 'n': runMode=MODE_AVOID; break;


    // STOP GLOBAL
    case 'i': case 'j': case 'k': case 'l':
      Motor(Stop,0,0,0,0); runMode=MODE_MANUAL; break;


    default:
      Motor(Stop,0,0,0,0);
      break;
  }
}


/* --------- Setup --------- */
void setup(){
  Serial.begin(115200);
  bt.begin(9600);


  pinMode(DIR_CLK,OUTPUT); pinMode(DATA,OUTPUT);
  pinMode(DIR_EN,OUTPUT); pinMode(DIR_LATCH,OUTPUT);
  pinMode(PWM0B,OUTPUT); pinMode(PWM0A,OUTPUT);
  pinMode(PWM2A,OUTPUT); pinMode(PWM2B,OUTPUT);


  pinMode(Trig,OUTPUT);
  pinMode(Echo,INPUT);


  digitalWrite(DIR_EN,LOW);
  Motor(Stop,0,0,0,0);


  Serial.println("Robot prêt !");
}


/* --------- Loop --------- */
void loop(){


  while(bt.available()){
    char c = bt.read();
    doCmd(c);
  }


  switch(runMode){
    case MODE_FOLLOW: Ultrasonic_Follow(); break;
    case MODE_AVOID: Ultrasonic_Avoidance(); break;
    default: break;
  }
}