
  #include <Wire.h>
  #include <OneButton.h>
  #include <Encoder.h>

  const int     storePressTimeMS  = 2000; 
  const int     I2CAddress        = 20;
  const String  I2CDataDelimiter  = "|";
  const String  I2CKeyDelimiter   = "#";

  OneButton memButton1(A0, true);
  OneButton memButton2(A1, true);
  OneButton memButton3(A2, true); 
  OneButton memButton4(A3, true);
  // A4 and A are used for I2C
  // A6 and A7 are not usable for ouput, they are only analog inputs
  //OneButton memButton5(A7, true);

  OneButton volumeRotaryButton(7, true);
  OneButton tuneRotaryButton(6, true);

  Encoder volumeRotaryEncoder(0, 1);
  Encoder tuneRotaryEncoder(3, 5);
  
  int ledPin_memButton1 = 12;
  int ledPin_memButton2 = 11;
  int ledPin_memButton3 = 10;
  int ledPin_memButton4 = 9;
  // Digital output 8 is broken on some boards, you have to reburn the Bootloader taht it works proper
  //int ledPin_memButton5 = 8;

  int memButton1_longPressStartMS = 0;
  int memButton2_longPressStartMS = 0;
  int memButton3_longPressStartMS = 0;
  int memButton4_longPressStartMS = 0;
  int memButton5_longPressStartMS = 0;
  int tuneButton_longPressStartMS = 0;
  int volumeButton_longPressStartMS = 0;

  long rotaryPos_Volume = 0;
  long rotaryPos_Tune   = 0;

  int lastMemButtonPressed      = 0;
  int lastMemButtonLongPressed  = 0;

  int tuneButtonPressed           = 0;
  int volumeButtonPressed         = 0;
  int tuneButtonDoubleClicked     = 0;
  int volumeButtonDoubleClicked   = 0;
  int tuneButtonLongPressed       = 0;
  int volumeButtonLongPressed     = 0;
  
  bool masterOnline = false;
  bool requestDone = false;

  // some vars for LED dancing
  int danceLedState = false;   
  unsigned long dancePreviousMillis = 0;   
  const long danceInterval = 200;  

  String I2CReturnString;
  String I2CReturnStringBuildHelper;
    
  void setup()                   
  {     
    Wire.begin(I2CAddress);
    Wire.onReceive(I2CReceive);
    Wire.onRequest(I2CRequest);
    
    Serial.begin(9600); // only for testing purposes
    
    pinMode(ledPin_memButton1, OUTPUT);     
    pinMode(ledPin_memButton2, OUTPUT);
    pinMode(ledPin_memButton3, OUTPUT);
    pinMode(ledPin_memButton4, OUTPUT);
    //pinMode(ledPin_memButton5, OUTPUT);
 
    memButton1.attachClick(memButton1_clicked);      
    memButton2.attachClick(memButton2_clicked);      
    memButton3.attachClick(memButton3_clicked);      
    memButton4.attachClick(memButton4_clicked);  
    //memButton5.attachClick(memButton5_clicked); 

    volumeRotaryButton.attachClick(volumeRotaryButton_clicked);      
    tuneRotaryButton.attachClick(tuneRotaryButton_clicked); 

    volumeRotaryButton.attachDoubleClick(volumeRotaryButton_doubleClicked);
    tuneRotaryButton.attachDoubleClick(tuneRotaryButton_doubleClicked);

    memButton1.attachLongPressStart(memButton1_longPressStart);
    memButton2.attachLongPressStart(memButton2_longPressStart);
    memButton3.attachLongPressStart(memButton3_longPressStart);
    memButton4.attachLongPressStart(memButton4_longPressStart);
    //memButton5.attachLongPressStart(memButton5_longPressStart);
    volumeRotaryButton.attachLongPressStart(volumeRotaryButton_longPressStart);
    tuneRotaryButton.attachLongPressStart(tuneRotaryButton_longPressStart);
    
    memButton1.attachDuringLongPress(memButton1_longPress);
    memButton2.attachDuringLongPress(memButton2_longPress);
    memButton3.attachDuringLongPress(memButton3_longPress);
    memButton4.attachDuringLongPress(memButton4_longPress);
    //memButton5.attachDuringLongPress(memButton5_longPress);
    volumeRotaryButton.attachDuringLongPress(volumeRotaryButton_longPress);
    tuneRotaryButton.attachDuringLongPress(tuneRotaryButton_longPress);

    memButton1.attachLongPressStop(memButton1_longPressStop);
    memButton2.attachLongPressStop(memButton2_longPressStop);
    memButton3.attachLongPressStop(memButton3_longPressStop);
    memButton4.attachLongPressStop(memButton4_longPressStop);
    //memButton5.attachLongPressStop(memButton5_longPressStop);
    volumeRotaryButton.attachLongPressStop(volumeRotaryButton_longPressStop);
    tuneRotaryButton.attachLongPressStop(tuneRotaryButton_longPressStop);

    buildI2CReturnString(); 
   
  }
   
  void loop()                     
  {
    if(requestDone)    
    {
      requestDone = false;
      buildI2CReturnString();
    }
    
    if(masterOnline)
    {
      memButton1.tick();
      memButton2.tick();
      memButton3.tick();
      memButton4.tick();
      //memButton5.tick();
      volumeRotaryButton.tick();
      tuneRotaryButton.tick();
      handleRotary();
      delay(10);    
    }
    else
    {
      // do some simple LED "dancing" while master system is not ready (booted)
      // we will get the "READY" / "UNREADY" sign via I2C from the master
      ledDance();
    }
  }

  void ledDance()
  {
    unsigned long currentMillis = millis();
    if (currentMillis - dancePreviousMillis >= danceInterval) 
    {  
      dancePreviousMillis = currentMillis;
      danceLedState = !danceLedState;        
      setLedPin(ledPin_memButton1, danceLedState);
      setLedPin(ledPin_memButton2, danceLedState);
      setLedPin(ledPin_memButton3, danceLedState);
      setLedPin(ledPin_memButton4, danceLedState);  
      //setLedPin(ledPin_memButton5, danceLedState); 
    }
  }

  void handleRotary()
  {
    long rotaryPos_VolumeNew, rotaryPos_TuneNew;
    rotaryPos_VolumeNew = (volumeRotaryEncoder.read() / 4);
    rotaryPos_TuneNew   = (tuneRotaryEncoder.read() / 4);

    if (rotaryPos_VolumeNew != rotaryPos_Volume || rotaryPos_TuneNew != rotaryPos_Tune) 
    {      
      rotaryPos_Volume  = rotaryPos_VolumeNew;
      rotaryPos_Tune    = rotaryPos_TuneNew;
      buildI2CReturnString(); 
    }    
  }


  // handle requests from master. we have to be sure that we do not do time intense stuff here becvause of
  // the I2C timing (interrupts). Otherwise it wont work!
  void I2CRequest() 
  {     
    // there may be a problem of buffer overrun if we send more than 32bytes so we try to keep the
    // sent value lower than that. Otherwise we may use something like the following commented code
    // but there we have to change the master code which runs on the rpi too...
    /*
      for (int x = 0; x < 194; x++)
      {
        Wire.beginTransmission (ADDRESS);
        Wire.send (x);
        Wire.endTransmission ();
      }
    */
    char *p = const_cast<char*>(I2CReturnString.c_str());
    Wire.write(p);     
   
    // other than the rotary tick values we have to reset the mem, pressed and long pressed state 
    lastMemButtonPressed        = 0;
    lastMemButtonLongPressed    = 0;
    volumeButtonPressed         = 0;
    tuneButtonPressed           = 0;
    tuneButtonDoubleClicked     = 0;
    volumeButtonDoubleClicked   = 0;
    tuneButtonLongPressed       = 0;
    volumeButtonLongPressed     = 0;
    requestDone                 = true;
  }

  void I2CReceive(int howMany)
  {
    int cmdRcvd = -1;  
    if (Wire.available())
    {
      // receive first byte there the command is, we keep it simple
      cmdRcvd = Wire.read();   
      switch(cmdRcvd)
      {
        case 0: lowButtonLeds(); break;
        case 1: highButtonLedPin(buttonIdToLedPin(1), true); break;
        case 2: highButtonLedPin(buttonIdToLedPin(2), true); break;
        case 3: highButtonLedPin(buttonIdToLedPin(3), true); break;
        case 4: highButtonLedPin(buttonIdToLedPin(4), true); break;
        //case 5: highButtonLedPin(buttonIdToLedPin(5), true); break;
        // when master comes online we have to set back the rotary encoders to prevent volume skiping (when encoder is moved while master is offline)     
        case 98:  rotaryPos_Volume = 0;
                  rotaryPos_Tune = 0;
                  volumeRotaryEncoder.write(0);
                  tuneRotaryEncoder.write(0);
                  buildI2CReturnString();
                  lowButtonLeds(); 
                  masterOnline = true; 
                  break;
        case 99: masterOnline = false;  lowButtonLeds(); break;
      }
    }   
}

  void buildI2CReturnString()
  { 
    
    I2CReturnStringBuildHelper = "";
    addI2CRequestValue("", String(lastMemButtonLongPressed));  
    addI2CRequestValue("", String(lastMemButtonPressed)); 
    addI2CRequestValue("", String(rotaryPos_Volume));  
    addI2CRequestValue("", String(rotaryPos_Tune));   
    addI2CRequestValue("", String(volumeButtonPressed));
    addI2CRequestValue("", String(tuneButtonPressed));

    addI2CRequestValue("", String(volumeButtonDoubleClicked));
    addI2CRequestValue("", String(tuneButtonDoubleClicked));
    addI2CRequestValue("", String(volumeButtonLongPressed));
    addI2CRequestValue("", String(tuneButtonLongPressed));
    
    I2CReturnStringBuildHelper += "§";

    // be sure that "I2CReturnString" is only touched a short time
    // and the other thing is we have to be sure that the interrupts are disabled while copying the string
    uint8_t SaveSREG = SREG;   // save interrupt flag
    cli();                     // disable interrupts    
    I2CReturnString = I2CReturnStringBuildHelper;
    SREG = SaveSREG;          // restore the interrupt flag
  }

  void lowButtonLeds()
  {
    digitalWrite(ledPin_memButton1, LOW);
    digitalWrite(ledPin_memButton2, LOW);
    digitalWrite(ledPin_memButton3, LOW);
    digitalWrite(ledPin_memButton4, LOW);
    //digitalWrite(ledPin_memButton5, LOW);
  }

  void addI2CRequestKeyValue(String _key, String _value)
  {
    if(I2CReturnStringBuildHelper != "")
    {
      I2CReturnStringBuildHelper += I2CDataDelimiter;
    }
    I2CReturnStringBuildHelper += _key + I2CKeyDelimiter + _value;      
  }

  void addI2CRequestValue(String _key, String _value)
  {
    if(I2CReturnStringBuildHelper != "")
    {
      I2CReturnStringBuildHelper += I2CDataDelimiter;
    }
    I2CReturnStringBuildHelper += _value;      
  }

  int buttonIdToLedPin(int _buttonId)
  {           
    switch (_buttonId)
    {
      case 1: return ledPin_memButton1;
      case 2: return ledPin_memButton2;
      case 3: return ledPin_memButton3;
      case 4: return ledPin_memButton4;
      //case 5: return ledPin_memButton5;
      default: 13;
    }
  }

  void highButtonLedPin(int _pin, bool _lowOtherLEDs)
  {     
    if(_lowOtherLEDs)
    { 
      lowButtonLeds();
    }
    setLedPin(_pin, true);
    //digitalWrite(_pin, HIGH);
  }

  void setLedPin(int _pin, bool _high)
  {     
    digitalWrite(_pin, _high);
  }

  void highButtonLed(int _buttonId, bool _lowOtherLEDs)
  {        
    highButtonLedPin(buttonIdToLedPin(_buttonId), _lowOtherLEDs);
  }

  bool isStorePressTimeReached(int _value)
  {
    if(_value && millis() - _value > storePressTimeMS) 
      return true;
    return false;
  }


  void action_StoreCurrentStream(int _button)
  {  
    lastMemButtonLongPressed = _button;
    buildI2CReturnString();
  }

  void action_SelectMemStream(int _button)
  {      
    lastMemButtonPressed = _button;   
    buildI2CReturnString();
  }

  void volumeRotaryButton_clicked() 
  {
    volumeButtonPressed = 1;
    buildI2CReturnString();
  }

  void volumeRotaryButton_doubleClicked() 
  {
    volumeButtonDoubleClicked = 1;
    buildI2CReturnString();
  }

  void tuneRotaryButton_clicked() 
  {
    tuneButtonPressed = 1;
    buildI2CReturnString();
  }

  void tuneRotaryButton_doubleClicked() 
  {
    tuneButtonDoubleClicked  = 1;
    buildI2CReturnString();
  }

  void memButton1_clicked() 
  {
    highButtonLedPin(ledPin_memButton1, true);   
    action_SelectMemStream(1);
  } 

  void memButton2_clicked() 
  {
    highButtonLedPin(ledPin_memButton2, true);
    action_SelectMemStream(2);
  } 

  void memButton3_clicked() 
  {
    highButtonLedPin(ledPin_memButton3, true);
    action_SelectMemStream(3);
  } 

  void memButton4_clicked() 
  {
    highButtonLedPin(ledPin_memButton4, true);
    action_SelectMemStream(4);
  } 

  /*
  void memButton5_clicked() 
  {
    highButtonLedPin(ledPin_memButton5, true);
    action_SelectMemStream(5);
  } 
  */


  void memButton1_longPressStart() 
  {
    memButton1_longPressStartMS = millis();
  } 

  void memButton2_longPressStart() 
  {
    memButton2_longPressStartMS = millis();
  } 

  void memButton3_longPressStart() 
  {
    memButton3_longPressStartMS = millis();
  } 

  void memButton4_longPressStart() 
  {
    memButton4_longPressStartMS = millis();
  } 

  /*
  void memButton5_longPressStart() 
  {
    memButton5_longPressStartMS = millis();
  } 
  */

  void volumeRotaryButton_longPressStart() 
  {
    volumeButton_longPressStartMS = millis();
  }

  void tuneRotaryButton_longPressStart() 
  {
    tuneButton_longPressStartMS = millis();
  } 


  void memButton1_longPressStop() 
  {
    memButton1_longPressStartMS = 0;
  } 

  void memButton2_longPressStop() 
  {
    memButton2_longPressStartMS = 0;
  } 

  void memButton3_longPressStop() 
  {
    memButton3_longPressStartMS = 0;
  } 

  void memButton4_longPressStop() 
  {
    memButton4_longPressStartMS = 0;
  } 

  /*
  void memButton5_longPressStop() 
  {
    memButton5_longPressStartMS = 0;
  } 
  */

  void volumeRotaryButton_longPressStop() 
  {
    volumeButton_longPressStartMS = 0;
  }

  void tuneRotaryButton_longPressStop() 
  {
    tuneButton_longPressStartMS = 0;
  } 


  void memButton1_longPress()   
  {
    if(isStorePressTimeReached(memButton1_longPressStartMS))   
    {    
      memButton1_longPressStartMS = 0;
      action_StoreCurrentStream(1);
      highButtonLedPin(ledPin_memButton1, true);
    }    
  }

  void memButton2_longPress()   
  {
    if(isStorePressTimeReached(memButton2_longPressStartMS))   
    {   
      memButton2_longPressStartMS = 0; 
      action_StoreCurrentStream(2);
      highButtonLedPin(ledPin_memButton2, true);
    }    
  }

  void memButton3_longPress()   
  {
    if(isStorePressTimeReached(memButton3_longPressStartMS))   
    {    
      memButton3_longPressStartMS = 0;
      action_StoreCurrentStream(3);
      highButtonLedPin(ledPin_memButton3, true);
    }    
  }

  void memButton4_longPress()   
  {
    if(isStorePressTimeReached(memButton4_longPressStartMS))   
    {    
      memButton4_longPressStartMS = 0;
      action_StoreCurrentStream(4);
      highButtonLedPin(ledPin_memButton4, true);
    }    
  }

  /*
  void memButton5_longPress()   
  {
    if(isStorePressTimeReached(memButton5_longPressStartMS))   
    {    
      memButton5_longPressStartMS = 0;
      action_StoreCurrentStream(5);
      highButtonLedPin(ledPin_memButton5, true);
    }    
  }
  */

  void volumeRotaryButton_longPress() 
  {
    if(isStorePressTimeReached(volumeButton_longPressStartMS))   
    {    
      volumeButton_longPressStartMS = 0;
      volumeButtonLongPressed = 1;
      buildI2CReturnString();
    }   
  }

  void tuneRotaryButton_longPress() 
  {
    if(isStorePressTimeReached(tuneButton_longPressStartMS))   
    {    
      tuneButton_longPressStartMS = 0;
      tuneButtonLongPressed = 1;
      buildI2CReturnString();
    }   
  } 




  



