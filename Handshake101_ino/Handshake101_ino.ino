
//this is the BASIC version of handshaking.
//Arduino starts a connection, prints 'a' with a counter over and over
//if there is a message in its Serial inbox, it acknowledges and
//Serial.println 's the message back, then closes connection.

const int returnMsgs=5;
int readStringCount = 0;
int maxStringCount = 100;
char ardString[8] = {'U', 'n', 'o', 'L', 'o', 'o', 'p', '\n'};

void setup(){
  Serial.begin(115200);
  //while(!Serial){;} dont freak out before connection made
  //doesnt seem necessary for Uno
  const int digiSensor = 2;
  pinMode(digiSensor, INPUT);

  sendPing();
  Serial.print('r');
}

void loop(){

  if (Serial.available()>0) {
    String inString  =String(Serial.readString());
    Serial.println('b');
    Serial.println(ardString);
    Serial.println(inString);
    //Serial.println(String(inString));
    //Serial.print('b' + String(inString));  
    for (int j=0; j<returnMsgs; j++)
    {
      Serial.println('t' + String(j));
      delay(1);
    }
    Serial.println('x');
    readStringCount ++;
  }
 // if (readStringCount >= maxStringCount || Serial.available() <= 0){
    if (readStringCount >= maxStringCount){      
      Serial.println('Closing Connection');
      Serial.end(); 
    }
    delay(10);
    
}

// good to remember- in Arduino, Serial.print() floats default to 
//two decimal places.
void sendPing(){
 //how specifically does the Serial.available call work?
 int pingCount = 0;
 int maxPings = 50;
 int tenFlipped = 0;
 int hundFlipped = 0;
 char header = 'a';
 //char nowChar[4];
 //char nowTime[3];
 while( (Serial.available()<=0 )  && (pingCount < maxPings) ){
   String nowString = String(String('a') + String(pingCount)); 
   //char printing routine
 
   //if (pingCount == 10) { tenFlipped = 1;}
   //if (pingCount == 100) {hundFlipped = 1;}
   //if (hundFlipped == 1){ nowTime = pingCount;}
   //else { if( tenFlipped == 1){nowTime='0' + pingCount};
   //else { nowTime = '00' + pingCount; }}
   //nowChar= header + nowTime + '\0';
   //
   //Serial.println('a');
   //Serial.write('s');
   Serial.println(nowString);
   delay (1000);
   pingCount ++;
 }
  
}
