// necessaire pour acceder à la fonction
void DisplayDrawInformations();

//*********************IMPORT from .ino original
void Display_X_Informations(); 
void Display_Y_Informations();
void Display_C_Informations();
void Display_M_Informations();
void Display_Extra_Informations();
void Display_Debug_Informations();
void Display_Notice_Informations(char* str);

//Menu item ******************************************
GEMPage menuPageShortcuts(""); // Shortcuts submenu
GEMItem menuItemShortcuts("", menuPageShortcuts);
GEMItem menuItemButtonShortcutsResetX("X = 0", ActionShortcutsResetX);
GEMItem menuItemButtonShortcutsResetY("Y = 0", ActionShortcutsResetY);
GEMItem menuItemButtonShortcutsResetM1("M1 = 0", ActionShortcutsResetM1);
GEMItem menuItemButtonShortcutsSetPosToMax("M1 -> M1 max", ActionShortcutsSetCurrentToMax);
GEMItem menuItemButtonShortcutsSetPosToMin("M1 -> M1 min", ActionShortcutsSetCurrentToMin);
GEMItem menuItemButtonShortcutsM1inManual("", ActionShortcutsM1inManual);
GEMItem menuItemButtonShortcutsM1inAuto("", ActionShortcutsM1inAuto);
GEMPage menuPageSettings(""); // Settings submenu
GEMItem menuItemMainSettings("", menuPageSettings);
GEMItem menuItemDirX("", sGeneralConf.Inverted_X,ActionChangeDirX);
GEMItem menuItemDirY("", sGeneralConf.Inverted_Y,ActionChangeDirY);
GEMItem menuItemDirZ("", sGeneralConf.Inverted_C,ActionChangeDirZ);
GEMItem menuItemDiamY("", sGeneralConf.Diameter_Mode_Y,ActionChangeDiamY);
GEMItem menuItemResoX("", sGeneralConf.Reso_X,ActionChangeResoX);
GEMItem menuItemResoY("", sGeneralConf.Reso_Y,ActionChangeResoY);
GEMItem menuItemResoZ("", sGeneralConf.Reso_C,ActionChangeResoZ);
GEMItem menuItemDirM1("", sGeneralConf.Inverted_M1,ActionChangeDirM1);
GEMItem menuItemResoM1("", sGeneralConf.Reso_M1,ActionChangeResoM1);
GEMItem menuItemThreadM1("", sGeneralConf.thread_M1,ActionChangeThreadM1);
GEMItem menuItemAccelM1("", sGeneralConf.Accel_M1,ActionChangeAccelM1);
GEMItem menuItemSpeedM1("", sGeneralConf.Speed_M1,ActionChangeSpeedM1);
SelectOptionByte selectLangOptions[] = {{"Fr", LANG_FR}, {"Eng", LANG_EN}};
GEMSelect selectLang(sizeof(selectLangOptions)/sizeof(SelectOptionByte), selectLangOptions);
GEMItem menuItemLang("", sGeneralConf.Lang, selectLang, ActionChangeLang);
GEMItem menuItemUseUSB("", sGeneralConf.UseUSBFunctions, ActionChangeUseUSB);
GEMItem menuItemButtonRestoreSettings("", ActionRestoreSettingsInFlash);
GEMItem menuItemButtonSaveSettings("", ActionSaveSettingsInFlash);

#ifdef JF_Snake
GEMItem menuItemButtonSnakeGame("Snake game !", ActionLaunchSnakeGame);
#endif

GEMItem menuItemButtonDro("", ActionDro);
GEMPage menuPageMain(TEXT_MAIN_MENU_TITLE);
GEMPage menuPageDebug(""); // Debug submenu
GEMItem menuItemDebug("", menuPageDebug);
GEMItem menuItemButtonDebug("Debug screen", ActionDebug);
GEMItem menuItemTestFloat("Float", TestFloat);
GEMPage menuPageAxe(""); // Axe submenu
GEMItem menuItemAxe("", menuPageAxe);
SelectOptionByte selectToolOptions[] = {{"Ref_0", 0}, {"Tool_1", 1}, {"Tool_2", 2}, {"Tool_3", 3}, {"Tool_4", 4}, {"Tool_5", 5}};
GEMSelect selectTool(sizeof(selectToolOptions)/sizeof(SelectOptionByte), selectToolOptions);
GEMItem menuItemTool("", bToolChoose, selectTool, applyTool);
GEMItem menuItemRelativeMode("", bRelativeModeActived,ActionChangeRelaticeMode);
GEMItem menuItemButtonResetX("X = 0", ActionResetX);
GEMItem menuItemButtonResetY("Y = 0", ActionResetY);
GEMItem menuItemAxeXPos("X = ?", fAxeXPos,ActionAxeXPos);
GEMItem menuItemAxeYPos("Y = ?", fAxeYPos,ActionAxeYPos);
GEMPage menuPageMotor(""); // Motor submenu
GEMItem menuItemMotor("", menuPageMotor);
GEMItem menuItemUseMotor("", bUseMotor,ActionUseMotor);
SelectOptionByte selectMotorModeOptions[] = {{"------", 0}, {"MANU", 1},{"AUTO", 2},{"TH EX N", 3},{"TH EX I", 4},{"TH IN N", 5},{"TH IN I", 6}};
GEMSelect selectMotorMode(sizeof(selectMotorModeOptions)/sizeof(SelectOptionByte), selectMotorModeOptions);
GEMItem menuItemMotorMode("Mode", bMotorMode, selectMotorMode, applyMotorMode);
GEMItem menuItemMotorStopMin("M1 min = ?", fMotorStopMin,ActionMotorStopMin);
GEMItem menuItemMotorStopMax("M1 max = ?", fMotorStopMax,ActionMotorStopMax);
GEMItem menuItemUseMotorEndLimit("", bUseMotorEndLimit,ActionUseMotorEndLimit);
GEMItem menuItemMotorCurrentPos("M1 = ?", fMotorCurrentPos,ActionMotorCurrentPos);
GEMItem menuItemMotorSpeed("", iMotorSpeed,ActionMotorMotorSpeed);
GEMItem menuItemButtonSetPosToMax("M1 -> M1 max", ActionSetCurrentToMax);
GEMItem menuItemButtonSetPosToMin("M1 -> M1 min", ActionSetCurrentToMin);
GEMItem menuItemButtonResetCurrentPos("M1 = 0", ActionResetCurrentPos);
GEMPage menuPageThreadParameters(""); // Thread parameters submenu
GEMItem menuItemThreadParameters("", menuPageThreadParameters);
GEMItem menuItemMotorThread("", iMotorThread,ActionMotorChangeThread);
GEMItem menuItemMotor1ThreadOffset("", fMotor1ThreadOffset,ActionChangeMotor1Offset);
GEMItem menuItemMotor1ThreadUseY("", bMotor1ThreadUseY,ActionMotor1ThreadUseY);
GEMItem menuItemMotor1ThreadDiameter("", fMotor1ThreadDiameter,ActionChangeMotor1ThreadDiameter);
GEMItem menuItemMotor1ThreadAngle("", fMotor1ThreadAngle,ActionChangeMotor1ThreadAngle);
GEMItem menuItemMotor1ThreadInfo("Vmax", fM1MaxThreadSpeed,true);
GEMItem menuItemMotorIncOffset("", ActionIncMotor1Offset);
GEMItem menuItemMotorDecOffset("", ActionDecMotor1Offset);
GEMPage menuPageProfilParameters(""); // Profil submenu
GEMItem menuItemProfil("", menuPageProfilParameters);
SelectOptionByte selectScreenOptions[] = {{"DroXYC", 0}, {"Mot1", 1}, {"Debug", 2}};
GEMSelect selectScreenMode(sizeof(selectScreenOptions)/sizeof(SelectOptionByte), selectScreenOptions);
GEMItem menuItemScreenMode("", eScreenChoose, selectScreenMode, ActionScreenMode);
