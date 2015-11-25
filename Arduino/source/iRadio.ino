
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

  Encoder volumeRotaryEncoder(2, 4);
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

  long rotaryPos_Volume = 0;
  long rotaryPos_Tune   = 0;

  int lastMemButtonPressed      = 0;
  int lastMemButtonLongPressed  = 0;
  
  bool masterOnline = false;

  // some vars for LED dancing
  int danceLedState = false;   
  unsigned long dancePreviousMillis = 0;   
  const long danceInterval = 200;  

  String I2CReturnString;
    
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

    memButton1.attachLongPressStart(memButton1_longPressStart);
    memButton2.attachLongPressStart(memButton2_longPressStart);
    memButton3.attachLongPressStart(memButton3_longPressStart);
    memButton4.attachLongPressStart(memButton4_longPressStart);
    //memButton5.attachLongPressStart(memButton5_longPressStart);
    
    memButton1.attachDuringLongPress(memButton1_longPress);
    memButton2.attachDuringLongPress(memButton2_longPress);
    memButton3.attachDuringLongPress(memButton3_longPress);
    memButton4.attachDuringLongPress(memButton4_longPress);
    //memButton5.attachDuringLongPress(memButton5_longPress);

    memButton1.attachLongPressStop(memButton1_longPressStop);
    memButton2.attachLongPressStop(memButton2_longPressStop);
    memButton3.attachLongPressStop(memButton3_longPressStop);
    memButton4.attachLongPressStop(memButton4_longPressStop);
    //memButton5.attachLongPressStop(memButton5_longPressStop);
   
  }
   
  void loop()                     
  {
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
      buildI2CReturnString(); // only for test
    }    
  }


  void I2CRequest() 
  {
    Serial.println("I2CRequest");
    // when we are return values for our master we are creating the return string from the current last values of each button 
    buildI2CReturnString(); 
    char *p = const_cast<char*>(I2CReturnString.c_str());
    Wire.write(p);     
    // other than the rotary tick values we have to reset the mem and long pressed state 
    lastMemButtonPressed = 0;
    lastMemButtonLongPressed = 0;
  }

  void I2CReceive(int howMany)
  {
    Serial.println("I2CReceive");
    int cmdRcvd = -1;  
    if (Wire.available())
    {
      // receive first byte there the command is, we keep it simple
      cmdRcvd = Wire.read();   
      switch(cmdRcvd)
      {
        case 1: highButtonLedPin(buttonIdToLedPin(1), true); break;
        case 2: highButtonLedPin(buttonIdToLedPin(2), true); break;
        case 3: highButtonLedPin(buttonIdToLedPin(3), true); break;
        case 4: highButtonLedPin(buttonIdToLedPin(4), true); break;
        //case 5: highButtonLedPin(buttonIdToLedPin(5), true); break;
        case 98: masterOnline = true;   lowButtonLeds(); break;
        case 99: masterOnline = false;  lowButtonLeds(); break;
      }
    }   
}

  void buildI2CReturnString()
  { 
    I2CReturnString = "";
    addI2CRequestKeyValue("MEM", String(lastMemButtonLongPressed));  
    addI2CRequestKeyValue("STREAM", String(lastMemButtonPressed)); 
    addI2CRequestKeyValue("VOL", String(rotaryPos_Volume));  
    addI2CRequestKeyValue("TUNE", String(rotaryPos_Tune));
    Serial.println(I2CReturnString); 
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
    if(I2CReturnString != "")
    {
      I2CReturnString += I2CDataDelimiter;
    }
    I2CReturnString += _key + I2CKeyDelimiter + _value;      
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
  }

  void action_SelectMemStream(int _button)
  {      
    lastMemButtonPressed = _button;   
  }

  void volumeRotaryButton_clicked() 
  {
    // TODO: @@@
    Serial.println("Volume Rotary Button clicked"); 
  }

  void tuneRotaryButton_clicked() 
  {
    // TODO: @@@
    Serial.println("Tune Rotary Button clicked"); 
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




  


