
// ***************************************************************************************
// *** Display functions *****************************************************************
void Display_StartScreen() 
{
  u8g2.clear();
  u8g2.firstPage();
  u8g2.setFontPosTop();
  do 
  {
    u8g2.drawXBMP((u8g2.getDisplayWidth() - logoMiniThread_width) / 2, (u8g2.getDisplayHeight() - logoMiniThread_height) / 2, logoMiniThread_width, logoMiniThread_height, logoMiniThread_bits);
  } while (u8g2.nextPage());  
  delay(500); 
  do 
  {
    u8g2.setFont(u8g2_font_6x12_tr); // choose a suitable font
    u8g2.drawStr(0,55,TEXT_AUTHOR_SOFT);
    u8g2.drawStr(70,55,TEXT_VERSION_SOFT);   
  } while (u8g2.nextPage()); 
  delay(2000);
  u8g2.clear();
  u8g2.firstPage();
  MyMsg.DisplayMsg(GetTxt(Id_Msg_Start),Msg::Warning,3000);
}

void DisplayDrawInformations()
{
  u8g2.firstPage();
  u8g2.setFontPosTop();
  do 
  {
    Display_UpdateRealTimeData();
    if( eScreenChoose == SCREEN_DRO )
    {
      Display_X_Informations();
      Display_Y_Informations();    
      Display_C_Informations();
      Display_Extra_Informations();
    }
    else if( eScreenChoose == SCREEN_MOT1 )  
    {
      Display_X_Informations();
      Display_Y_Informations();    
      Display_M_Informations();
      Display_Extra_Informations();      
    }
    else if( eScreenChoose == SCREEN_DEBUG )  
    {
      Display_Debug_Informations(); 
    }   
  } while (u8g2.nextPage());
}

void Display_X_Informations()
{
  char bufferChar[16];
  u8g2.setColorIndex(1);
  u8g2.setFont(u8g2_font_profont22_tf); // choose a suitable font
  u8g2.drawStr(0,1,"X");
  u8g2.setColorIndex(1);
  sprintf(bufferChar,"%+09.3f",fAxeXPos);  
  u8g2.drawStr(13,1,bufferChar);  // write something to the internal memory
  u8g2.drawRFrame(11,0,116,18,3);  
}
void Display_Y_Informations()
{
  char bufferChar[16];
  u8g2.setColorIndex(1);
  u8g2.setFont(u8g2_font_profont22_tf); // choose a suitable font
  u8g2.drawStr(0,19,"Y");
  sprintf(bufferChar,"%+09.3f",fAxeYPos);
  u8g2.drawStr(13,19,bufferChar);  // write something to the internal memory
  u8g2.drawRFrame(11,18,116,18,3);    
}
void Display_C_Informations()
{
  char bufferChar[16];
  u8g2.setColorIndex(1); 
  u8g2.setFont(u8g2_font_profont22_tf); // choose a suitable font 
  u8g2.drawStr(0,37,"C");
  sprintf(bufferChar,"%+5.5d",(int)fAxeCSpeed);
  u8g2.drawStr(13,37,bufferChar);  // write something to the internal memory
  u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
  sprintf(bufferChar,"%07.3f",(float)Quad_C.GetValuePos()/sGeneralConf.Reso_C*360.0);
  u8g2.drawStr(85,37,bufferChar);  // write something to the internal memory
  u8g2.drawStr(85,45,"tr/min");
  u8g2.drawRFrame(11,36,116,18,3);    
}
void Display_M_Informations()
{
  char bufferChar[30];
  u8g2.drawStr(0,37,"M");
  u8g2.setColorIndex(1);
  if( bUseMotor == true )
  {
    u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
    sprintf(bufferChar,"%+09.3f",fMotorCurrentPos);
    u8g2.drawStr(13,37,bufferChar);  // write something to the internal memory
    switch ( bMotorMode )
    {
      case MOTOR_MODE_NO_MODE:
        u8g2.drawStr(57,37,"|------");
      break;
      case MOTOR_MODE_MANUAL:
        u8g2.drawStr(57,37,"|MANU");
      break;
      case MOTOR_MODE_AUTO:
        u8g2.drawStr(57,37,"|AUTO");
      break;
      case MOTOR_MODE_TH_EXT_N:
        u8g2.drawStr(57,37,"|THEX N"); 
      break;
      case MOTOR_MODE_TH_EXT_I:
        u8g2.drawStr(57,37,"|THEX I"); 
      break;
      case MOTOR_MODE_TH_INT_N:
        u8g2.drawStr(57,37,"|THIN N"); 
      break;
      case MOTOR_MODE_TH_INT_I:
        u8g2.drawStr(57,37,"|THIN I"); 
      break;      
    }
    //Motor speed
    //if motor is Left mode, display the speed from the settings (Max speed )
    sprintf(bufferChar,"|%d", (unsigned int)fM1ActualSpeed);
    //sprintf(bufferChar,"|%d",iMotorSpeed);
    u8g2.drawStr(95,37,bufferChar);    
    
    //End limit 
    sprintf(bufferChar,"%+09.3f <> %+09.3f",fMotorStopMax,fMotorStopMin);
    if(bUseMotorEndLimit)u8g2.drawStr(13,45,bufferChar);
    else u8g2.drawStr(16,45,GetTxt(Id_Msg_Motor_NoEndLimit));       
  }
  else
  {
    u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
    u8g2.drawStr(16,40,GetTxt(Id_Msg_Motor_Disabled));   
  }  
  u8g2.drawRFrame(11,36,116,18,3);    
}
void Display_Extra_Informations()
{
  char bufferChar[10];
  u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
  u8g2.drawStr(0,54,selectToolOptions[bToolChoose].name);
  //Display thread Masterstate 
  if (  bMotorMode == MOTOR_MODE_TH_EXT_N ||
        bMotorMode == MOTOR_MODE_TH_EXT_I ||
        bMotorMode == MOTOR_MODE_TH_INT_N ||
        bMotorMode == MOTOR_MODE_TH_INT_I )
  {
    switch(eMS_Thread)
    {
      case MS_THREAD_IDLE:
        u8g2.drawStr(30,54,"|IDLE");
      break;  
      case MS_THREAD_WAIT_THE_START:
        u8g2.drawStr(30,54,"|WAIT START");
      break;  
      case MS_THREAD_WAIT_THE_SPLINDLE_ZERO:
        u8g2.drawStr(30,54,"|WAIT SYNC");
      break;  
      case MS_THREAD_IN_THREAD:
        u8g2.drawStr(30,54,"|IN THREAD");
      break;  
      case MS_THREAD_END_THREAD:
        u8g2.drawStr(30,54,"|WAIT RETURN");
      break;             
      case MS_THREAD_IN_RETURN:
        u8g2.drawStr(30,54,"|IN RETURN");
      break;      
    }    
  }
  //Display Abs / relative for axe X and Y 
  if(bRelativeModeActived==true)u8g2.drawStr(108,54,"|Rel");
  else u8g2.drawStr(108,54,"|Abs");  
}
void Display_UpdateRealTimeData()
{
  fMotorCurrentPos = Motor1.GetPositionReal();
  fM1ActualSpeed = Motor1.GetMaxSpeed(); 
  fAxeXPos = Quad_X.GetValue();
  fAxeYPos = Quad_Y.GetValue();
  fAxeCSpeed = (float)Quad_C.GiveMeTheSpeed();     
}
void Display_Debug_Informations()
{
  char bufferChar[30];
  u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
  u8g2.drawStr(0,0,"Debug page !");
  sprintf(bufferChar,"I:%d",Motor1.NewInterval());
  u8g2.drawStr(0,9,bufferChar);  // write something to the internal memory
  sprintf(bufferChar,"_n:%ld",Motor1._n);
  u8g2.drawStr(0,18,bufferChar);  // write something to the internal memory
  sprintf(bufferChar,"_c0:%f",Motor1._c0);
  u8g2.drawStr(0,27,bufferChar);  // write something to the internal memory
  sprintf(bufferChar,"_cmin:%f",Motor1._cmin);
  u8g2.drawStr(0,36,bufferChar);  // write something to the internal memory
}

void Display_Notice_Informations(char* str)
{
  u8g2.firstPage();
  u8g2.setFontPosTop();
  do 
  {
    u8g2.drawRFrame(0,0 ,128,64,4);
    u8g2.setFont(u8g2_font_6x12_tr); // choose a suitable font
    u8g2.drawStr(3,3,str);
  } while (u8g2.nextPage());  
  delay(2000); 
}


// ***************************************************************************************

extern float       mem_fAxeXPos,mem_fAxeYPos,mem_fAxeCSpeed;
extern float       fAxeXPos,fAxeYPos,fAxeCSpeed;

// *** Usb Serial functions *****************************************************************
void UsbSerial_Pos()
{
  
  char bufferChar[30];
#ifndef ESP32
  if(Serial.isConnected())
#else
  if(Serial) //if(Serial.available())
#endif
  {
  //ESP32 print si changement etat d'au moins une valeur
  if ((fAxeXPos != mem_fAxeXPos)||(fAxeYPos != mem_fAxeYPos)||(fAxeCSpeed != mem_fAxeCSpeed))
    {  
    mem_fAxeXPos = fAxeXPos ;
    mem_fAxeYPos = fAxeYPos ;
    mem_fAxeCSpeed = fAxeCSpeed ;
    sprintf(bufferChar,"X%0.3f:",fAxeXPos); 
    Serial.print(bufferChar);
    sprintf(bufferChar,"Y%0.3f:",fAxeYPos); 
    Serial.print(bufferChar);
    sprintf(bufferChar,"C%0.3f",fAxeCSpeed); 
    Serial.print(bufferChar);
    Serial.print("\n");  
    } 
  }
}
