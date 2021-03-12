
// This code is licenced by it's creator Ben Jacobs (KC9ROI) under the CC BY-NC-SA 3.0 licence:
// You are allowed to change, copy, reditribute, transmit, and adapt (remix) this code, as long as you licence your change, or adaptation under
// this or a similar licence, and you do not use this code, or any adaptations/changes of it for commercial purposes without the author's permission.
// Thanks!

int holdTime = 500;
String holdCommand1 = "FA00003560000;";
String holdCommand2 = "FA00007040000;";
String holdCommand3 = "FA00010106000;";
String holdCommand4 = "FA00014060000;";
String holdCommand5 = "FA00018096000;";
String holdCommand6 = "FA00021060000;";
String holdCommand7 = "FA00001810000;";
String holdCommand8 = "FA00028060000;";
String message;
char inByte = ' ';

int tuneBtn = 12;
int macroBtn = 0;
int start = 3;
int key = 2;
int grnLED = A0;
int redLED = A1;
int serialSwitch = A2;

int tuneStatus; 
int lightStatus;

int tuneBtnState = 0;
int macroBtnState = 0;
int i;

unsigned long timestamp = 0;

void setup() 
{
  Serial.begin(19200);
  pinMode(tuneBtn, INPUT_PULLUP); // Button that activates the tune code
  pinMode(11, INPUT_PULLUP); // macro 1 ...
  pinMode(10, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP); // ... through macro 8
  pinMode(start, OUTPUT); //Start line (white wire from tuner)
  pinMode(grnLED, OUTPUT); //Green half of bi color LED
  pinMode(redLED, OUTPUT); //Red half of bi color LED
  pinMode(key, INPUT); //Key line (Green from tuner)
  digitalWrite(serialSwitch, LOW);
  digitalWrite(redLED, LOW);
  digitalWrite(grnLED, HIGH);
}

void loop() // Main loop 
{
  tuneBtnState = digitalRead(tuneBtn); // Read tune button
  if (tuneBtnState == LOW) //With internal pullups, LOW means the button has been pressed
  {
   delay(holdTime);
   tuneBtnState = digitalRead(tuneBtn);
   if (tuneBtnState == LOW)
   {
    digitalWrite(start, HIGH);
    delay(70);
    digitalWrite(start, LOW);
    lightStatus = 2;
   }
   else
   {
    delay(100);
    digitalWrite(serialSwitch, HIGH); //Set the serial line to arduino via the ADG419 analog switches
    delay(100);
    digitalWrite(start, HIGH);
    timestamp = millis();
    while (1 > 0)
    {
     if (millis() - timestamp >= 500)
     {
      digitalWrite(start, LOW); 
      lightStatus = 0;
      goto tuneEnd;  // Key line timed out. Check that the tuner is plugged in and try again.
     } 
     if (digitalRead(key) == HIGH) // Wait for key line to go high, indicating that the Rig should start Txing
     {
      digitalWrite(start, LOW); //KEY is high, turn off START
      break;
     }
    }
    
    // No errors so far, KEY was detected, start tuning operation 
    
    Serial.print("SWH16;"); //Enter tune mode, start TXing
    while(1 > 0)
    {
      if (digitalRead(key) == LOW) // Wait for KEY to go LOW
      {
        delay(120); // Wait 120ms
        if (digitalRead(key) == HIGH) // If after 120ms the KEY line is back HIGH again, a pulse has occoured (Bad tune status)
        {
        lightStatus = 0;

         break;
        }
        else
        {
         lightStatus = 1;

         break;
        }
      }
    }

    tuneEnd: // Tuning has finished, or an error has occoured causing a jump to this line. Clear serial buffer and query the K3 as to if it is TXing still or not. 
 
    while (Serial.available() > 0) // The folowing code clears the Arduino's serial buffer so that, in the event a user calls an unprogrammed (empty) macro, the ";" character(s)
    {                             // returned by the empty macro call are erased, as to ensure that the next command sent to the K3 returns a valid response.
     inByte = Serial.read();    
    }
  
    Serial.print("TQ;"); // Transmit query command
    message = "";
    timestamp = millis(); // Store the current time in a variable called timestamp
    while (1 > 0)
    {
     if (Serial.available() > 0) // If serial is available
     {
      inByte = Serial.read(); // Read one byte of it into inByte
      message += inByte; // Add inByte to message
     }
      if (message == "TQ1;") // If the K3 is still TXing (As it should be), tell it to stop and leave the loop.
      {
       Serial.print("SWT16;"); // Press the TX button (toggle on/off)
       lightStatus = 1;
       break;
      }
      if (message == "TQ0;") // If the K3 is not TXing, since I have not told it to stop yet, there must be an error. Turn on the RED (Error) led. (See next comment)
      {
       //lightStatus = 0; // Add in these two lines if you want the RED led come on. 
       //light();
       break;
      }
      if (millis() - timestamp >= 200) // If timestamp - the current time >= 200ms
      {          
       lightStatus = 0; // There was no response from the  k3, so turn on the error LED
       break;
      }
    
    }

   }
    light();
    delay(100);
    digitalWrite(serialSwitch, LOW); // switch off switch
    while (tuneBtnState == LOW)
    {
     tuneBtnState = digitalRead(tuneBtn);
     delay(5); 
    }
   }
   
   
   // Read macro buttons one after another in this FOR loop. Call macroFunction() if one is high to handle it.
   for (macroBtn = 4; macroBtn < 12; macroBtn += 1)
   {   
    macroBtnState = digitalRead(macroBtn);
    if (macroBtnState == LOW)
    {
      macroFunction();
    }
   }
}

void macroFunction()
{
  delay(holdTime);
  macroBtnState = digitalRead(macroBtn);
  digitalWrite(serialSwitch, HIGH); 

  if (macroBtnState == HIGH)
  {   
   switch (macroBtn)
   {
     case 4:
      Serial.print(holdCommand8);
      break;
     case 5:
      Serial.print(holdCommand7);
      break;
     case 6:
      Serial.print(holdCommand6);
      break;
     case 7:
      Serial.print(holdCommand5);
      break;
     case 8:
      Serial.print(holdCommand4);
      break;
     case 9:
      Serial.print(holdCommand3);
      break;
     case 10:
      Serial.print(holdCommand2);
      break;
     case 11:
      Serial.print(holdCommand1);
      break;
   } 
  }
  else
  {
   switch (macroBtn)
   {
     case 4:
      Serial.print("SWH39;");
      break;
     case 5:
      Serial.print("SWH35;");
      break;
     case 6:
      Serial.print("SWH31;");
      break;
     case 7:
      Serial.print("SWH21;");
      break;
     case 8:
      Serial.print("SWT39;");
      break;
     case 9:
      Serial.print("SWT35;");
      break;
     case 10:
      Serial.print("SWT31;");
      break;
     case 11:
      Serial.print("SWT21;");
      break;
   }
  }
  delay(100);
  digitalWrite(serialSwitch, LOW); // switch off switch
  while (macroBtnState == LOW)
  {
   macroBtnState = digitalRead(macroBtn);
   delay(5); 
  }
}

void light() // Function that changes the LED's color according to the contents of the "lightStatus" variable
{
 switch(lightStatus)
 {
   case 0: // 0 = red (Error/bad tune)
    digitalWrite(grnLED, LOW);
    digitalWrite(redLED, HIGH);
    i = 0;
    while (i < 4)
    {
    digitalWrite(redLED, LOW); 
    delay(200); 
    digitalWrite(redLED, HIGH);
    delay(200);
    i+=1;
    }
    break;
   case 1: // 1 = green (No errors/good tune)
    digitalWrite(redLED, LOW);
    digitalWrite(grnLED, HIGH);
    break;
   case 2: // 2 = orange (tuner out of circuit; "Un-tune")
    digitalWrite(redLED, HIGH);
    digitalWrite(grnLED, HIGH);
    break;
 }
}
