//                                            1         2
//                                   12345678901234567890
#define MSG_SAVED                   "Saved!"
#define MSG_CURRENTLY               "(currently"
#define MSG_BRACKET                 ")"
#define MSG_ENTER_PARAM_DELAY       "Enter the Delay:"
#define MSG_ENTER_PARAM_PWM_SPEED   "Enter the PWM speed:"

#define MSG_TAKING_PICTURES        "Taking pictures..."
#define MSG_OVER_6                 "(over 6 positions)"
#define MSG_READY                  "Moving to position..."
#define MSG_MOVING_BACK            "Moving back..."
#define MSG_DONE                   "Done!"

// ---------

#define N_MENU_LINES 4
const __FlashStringHelper* screen[N_MENU_LINES];

void menuDisplay() {
   for(byte i = 0; i < N_MENU_LINES; i++)
     lcdPrintLine(lcdLines[i], screen[i]); 
}

void menuDisplayHome() {
   //                      1         2
   //             12345678901234567890
   screen[0] = F("Welcome to UC Davis");
   screen[1] = F("  3D reconstruction");
   screen[2] = F("  system for plants");
   screen[3] = F("   [Slaughter's Lab]"); 
   menuDisplay();
}

void menuDisplayHelp() {
   //                      1         2
   //             12345678901234567890
   screen[0] = F("Press CLEAR to reset");
   screen[1] = F("Press SHIFT to set");
   screen[2] = F("        parameters");
   screen[3] = F("Press ENTER to start");
   menuDisplay();
}

void menuDisplayClear() {
   //                      1         2
   //             12345678901234567890
   screen[0] = F("Reset all settings");
   screen[1] = F("   and retract all");
   screen[2] = F("   actuators fully");
   screen[3] = F("   ...RUNNING...");
   menuDisplay();
}

void menuDisplaySetting() {
   //                      1         2
   //             12345678901234567890
   screen[0] = F("1: Set Delay to stop");
   screen[1] = F("   at each setpoint");
   screen[2] = F("2: Set PWM speed");
   screen[3] = F("");
   menuDisplay();
}
