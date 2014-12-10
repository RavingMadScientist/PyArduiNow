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
const int returnMsgs=5;
int readStringCount = 0;
int maxStringCount = 100;
//char ardString[8] = {'U', 'n', 'o', 'L', 'o', 'o', 'p', '\n'};

char* reqArgs[] = {"00000000", "11111111", "22222222", "33333333", "44444444", "55555555"};
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
    String orgString  =String(Serial.readString());
    Serial.println(orgString);
        
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
    Serial.println("comst");
    Serial.println(String(nextcomma));
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
          parsedString.toCharArray(reqArgs[reqSegs], 8) ;
        }
        else {
          inString.toCharArray(reqArgs[reqSegs], 8) ;
        }
        
        //iteration num, and clipped string for next iteration
        Serial.println(String("iter"));
        Serial.println(reqSegs);
        reqSegs++;
        
        //beginning string for iteration
        Serial.println("inString");
        Serial.println(inString);
        
        //beginning indexOf along which string is to be parsed
        Serial.println("comloc");
        Serial.println(String(nextcomma));
        
        //clipped string for next iteration, print 'before' and 'after'
        String newString=inString.substring(nextcomma+1);     
        Serial.println(String("news"));
        //Serial.println(inString);      
        Serial.println(newString);
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
 int serAv=Serial.available();

 while( ( serAv<=0 )  && (pingCount < maxPings) ){
   String nowString = String(String('a') + String(pingCount)); 
   Serial.println(nowString);
   delay (1000);
   serAv=Serial.available();
   pingCount ++;
 }
  
}
