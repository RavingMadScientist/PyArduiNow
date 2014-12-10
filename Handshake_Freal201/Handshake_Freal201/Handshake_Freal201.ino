//this is the v1.02 version of handshaking.
//Arduino starts a connection, prints 'a' with a counter over and over
//if there is a message in its Serial inbox, it acknowledges with 
//the char 'r'.

//then it waits for a valid reqString from Python.
//the default is 'b10,s,1E0r,g' . The reqString is valid as long as
//its last element (comma delimited) is 'g'


//Three functions are defined in this module:
//setup(). It begins with the custom-defined
//sendPing(), which sends a 'a' concated witha counter, which it emits
//at adjustable intervals

/* whenever the serial.available() function called each iteration
returns a >0 value (Bytesize), the while loop exits.
Then we return control to setup, which emits an 'r' and terminates.
Thus, the loop() infinite loop begins.

Every time serial.available() reads >0 (including the first time, since
a read is never called from sendPing),
 Arduino reads a string at a time (presumably space-delimited)
It will always respond to a recd string with a 'b' or 'l'.

Since data requests are always comma-delimited (specifying burst/cont,
internal clock status, rate request, and a trigger Byte,
Arduino tries to respond to all messages containing ',' with a data stream

So it gets indexOf ',' in the received string, 
and returns this value and resulting logic pathway in the form
"comst\n", indexOf\n, "b"/"l" for read / noread. if noread, then l returns a 
sequence of counts t0\n...t4\n

in 'b' (dataread) mode, Arduino parses the substring up to 1st comma, and
reads that into at char array Array, initialized to max of 6 8-byte entries
while this limit hasnt been exceeded.

Its message sequence for each iteration is "iter\n", ... , "inString\n" ,...,
"comloc\n", inString.indexOf(',')\n, "news\n", newString\n 

Finally, regardless of the return mode, Arduino sends an 'x' , which is 
known to Python as the transaction termination signal.


*/ 
#include <stdlib.h>
const int returnMsgs=5;
int readStringCount = 0;
int maxStringCount = 100;
//char ardString[8] = {'U', 'n', 'o', 'L', 'o', 'o', 'p', '\n'};

char* reqArgs[] = {"0000000000000000", "1111111111111111", "2222222222222222", "3333333333333333", "44444444", "55555555"};
int maxReqArgs = 4;
int burstNum;
float reqCoef;
int reqSig;


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
    //declare variables for data acq now, for quicker response when
    //requested, and give default vals
    int burstmode=1;
    long burstnum=20;
    int sendclock=0;
    int ratebool=0;
    float ratems=50;
    int delbool=0;
    float delms=0;
    int numpins=1;
    //multipin to be added later...
    String orgString  =String(Serial.readString());
    Serial.println(orgString);
    //adding reset functionality :)
    //'Z' is the reset string
    if (orgString.charAt(0)=='Z'){
      Serial.println("Arduino catches a RESET call Z");
      Serial.end();
      delay(1000);
      Serial.begin(115200);
      sendPing();
      Serial.print('r');
    }
    //the rest handles everything that is not a RESET request
        
    String inString = orgString;
    int goodstatus = 1;
    int reqSegs=0;  // this is Counter for # commas
    //check for valid reqString
    int nextcomma=inString.indexOf(',');
    //decide which report loop to enter based on this info
    if (nextcomma  < 0) {
      goodstatus = 0;}
    
    //relay comma status, and decide whether we enter 
    //data acq. loop, or 'echo' loop. data only returned for
    //properly formatted request from Python (with commas)
  //Serial.println("comst");
  //Serial.println(String(nextcomma));
    //Serial.println("rtyp");
    //Serial.println(String(goodstatus));
    //data loop
    if (goodstatus == 1){
      Serial.println('b');
      
      while ((nextcomma > -1) && (reqSegs < maxReqArgs)) {
        //comma delimiting, split into the char  ** [][]
        nextcomma=inString.indexOf(',');
        if (nextcomma>0){
          String parsedString=inString.substring(0,nextcomma);
          parsedString.toCharArray(reqArgs[reqSegs], 16) ;
        }
        else {
          inString.toCharArray(reqArgs[reqSegs], 16) ;
        }
        
        //iteration num, and clipped string for next iteration
      //Serial.println(String("iter"));
      //Serial.println(reqSegs);
        reqSegs++;
        
        //beginning string for iteration
      //Serial.println("inString");
      //Serial.println(inString);
        
        //beginning indexOf along which string is to be parsed
      //Serial.println("comloc");
      //Serial.println(String(nextcomma));
        
        //clipped string for next iteration, print 'before' and 'after'
        String newString=inString.substring(nextcomma+1);     
      //Serial.println(String("news"));
        //Serial.println(inString);      
      //Serial.println(newString);
        inString = newString;  
      }
      //output for data loop in H102 is the parsed entries of the
      //string array reqArgs
      for (int j=0; j<reqSegs; j++)
      {
        Serial.println('u' + String(j));
       Serial.println(String(reqArgs[j]));
        delay(1);
      }
      //for the REAL exchange, we first simulate 'datamode' messages
      //and gradually work in the reqString parsing functions
      
      //first, lets use the first argument, b#/c, to return the
      //requested number of datapoints
      
      if (reqArgs[0][0]=='b'){
        burstmode=1;
        String burststring=String(reqArgs[0]);
        burststring=burststring.substring(1);
        burstnum=burststring.toInt();
      }
      else{burstnum=999999999999L; 
        if (reqArgs[0][0]=='c'){burstmode=0; }
        else{burstmode=-1;} }

      //next sendclock
      if (reqArgs[1][0]=='s'){
        sendclock=1;}
      // now ratestuff
      String rString=String(reqArgs[2]);
      int RateStrLg=rString.length();
      int delloc=rString.indexOf('d');
      int ratestart=0;
      if (delloc>0){ delbool=1;}
      char rchar=reqArgs[2][RateStrLg-1];
      if (rchar=='r'){
         ratebool=1;
        }
      if (delbool>0){
          String delSciNum=rString.substring(0,delloc);
          ratestart=delloc+1;  
        int dEindex=delSciNum.indexOf('E');
        String dsnString=delSciNum.substring(0,dEindex);
        //Arduino is dumb about string/float conversions
        char dfloatbuf[32]; // make this at least big enough for the whole string
        dsnString.toCharArray(dfloatbuf, sizeof(dfloatbuf));
        float dcoef = atof(dfloatbuf);

        int dexp=delSciNum.substring(dEindex+1).toInt();
        delms=dcoef*pow(10.0,dexp);
      }
      else{
        delms=0;
      }
      if (ratebool>0){
        String rateSciNum=rString.substring(ratestart, RateStrLg-1);
        int rEindex=rateSciNum.indexOf('E');
        String rsnString=rateSciNum.substring(0,rEindex);
        //Arduino is dumb about string/float conversions
        char rfloatbuf[32]; // make this at least big enough for the whole string
        rsnString.toCharArray(rfloatbuf, sizeof(rfloatbuf));
        float rcoef = atof(rfloatbuf);

        int rexp=rateSciNum.substring(rEindex+1).toInt();
        ratems=rcoef*pow(10.0, rexp);
      }
      else{ 
        ratems=50;
      }
      String gostring=String(reqArgs[3]);

      //Arduino is dumb about printing floats
      char rmschar[8];
      dtostrf(ratems, 4, 3, rmschar);
      String rmsString="";
      for(int i=0;i<sizeof(rmschar);i++)
      {
        rmsString+=rmschar[i];
      }     
      char dmschar[8];
      dtostrf(delms, 4, 3, dmschar);
      String dmsString="";
      for(int i=0;i<sizeof(dmschar);i++)
      {
        dmsString+=dmschar[i];
      }  
      //print params before the big show
      Serial.println(String("burstmode"));
      Serial.println(String(burstmode));
      Serial.println(String("burstnum"));
      Serial.println(String(burstnum));      
      Serial.println(String("sendclock"));
      Serial.println(String(sendclock));

      Serial.println(String("ratebool"));
      Serial.println(String(ratebool));
      Serial.println(String("ratems"));
      Serial.println(String(rmsString));
      Serial.println(String("delbool"));
      Serial.println(String(delbool)); 
      Serial.println(String("delms"));
      Serial.println(String(dmsString));      
      //Serial.println(String("delloc"));
      //Serial.println(String(delloc)); 
      //Serial.println(String("RateStrLg"));
      //Serial.println(String(RateStrLg));   
      //Serial.println(String("rchar"));
      //Serial.println(String(rchar));      
      Serial.println(String("gostring"));
      Serial.println(String(gostring)); 
      
      //here begins the REAL readin and writin loop
      //Arduino-specific, have to determine whether to use delay(ms)or delayMicro
      if (delms>10){
        delay(delms);}
      else{ if (delms>0){
        delayMicroseconds(delms*1000);
        }
      }
      // now begin acquisition
      int readCount=0;
      String pString;
      Serial.println('y');
      //slightly dif. loop for sendclock, for efficiency
      if (sendclock==1){
        long AcqStartTime=micros();
        long maxtime=AcqStartTime;
        long clocktime;
        long abstime;
        long resetAccum=0;
        while (readCount<burstnum){
          int thisread = analogRead(A0);
          abstime=micros();
          if (abstime>maxtime){
              clocktime=abstime+resetAccum-AcqStartTime;
              maxtime=abstime;
          }
          //event handling for micros() reset
          else{
            resetAccum+=maxtime;
            clocktime=abstime+resetAccum-AcqStartTime;
            maxtime=abstime;
          }
          
          pString="z"+String(clocktime)+";;"+String(thisread);
          Serial.println(pString);
          if (ratems>10){
            delay(ratems);
          }
          else{
            delayMicroseconds(ratems*1000); 
          }
          readCount++;       
      }
      }
      //non-sendclock just omits time-checking and string-formatting
      else{
        // and loop autodecides delay() or delayMicros() for speed&efficiency
        if (ratems>10){
          while (readCount<burstnum){
          int thisread = analogRead(A0);
          pString="z"+String(thisread);
          Serial.println(pString);
          delay(ratems);
          readCount++;
          }
        }
        else{
          float ratemics=ratems*1000;
          while (readCount<burstnum){
            int thisread = analogRead(A0);
            pString="z"+String(thisread);
            Serial.println(pString);
            readCount++;
            delayMicroseconds(ratemics);
          }
        
        }
      
      }
      
    }
//end data loop SerialPrinting function

//non-data (echo) loop SerialPrinting function: t0 t1 t2 t3 t4 t5
    else{
    Serial.println('l');
    for (int j=0; j<5; j++)
    {
      Serial.println('t' + String(j));
      //Serial.println(reqArgs[j]);
      delay(1);
    }
    }
    
    //terminate response to python signal
    Serial.println('x');
    readStringCount ++;
}
 // if (readStringCount >= maxStringCount || Serial.available() <= 0)
    if (readStringCount >= maxStringCount){      
      Serial.println('Closing Connection');
      Serial.end(); 
    }
    delay(10); //ms
    
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
 int serAv=Serial.available();

 while( ( serAv<=0 )  && (pingCount < maxPings) ){
   String nowString = String(String('a') + String(pingCount)); 
   Serial.println(nowString);
   delay (1000);
   serAv=Serial.available();
   pingCount ++;
 }
  
}

