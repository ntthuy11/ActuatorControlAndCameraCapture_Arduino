#define LCD_PIN         42     // digital pin for software serial
SoftwareSerial lcdSerial = SoftwareSerial(LCD_PIN, LCD_PIN); 

#define LCD_CMD_CLEAR   1      // command to clear the lcd screen
#define LCD_CMD_LISTEN  254    // command to tell LCD to listen

#define LCD_LINE_1      128    // first position for line 1
#define LCD_LINE_2      192 
#define LCD_LINE_3      148 
#define LCD_LINE_4      212
byte lcdLines[4] = {LCD_LINE_1, LCD_LINE_2, LCD_LINE_3, LCD_LINE_4};

// -------------------------------------------------------------------

void lcdClear() {
    lcdSerial.write(LCD_CMD_LISTEN); // command to tell LCD to listen
    lcdSerial.write(LCD_CMD_CLEAR);  // command to tell LCD to clear screen
}

void lcdClearLine(byte location) {
    lcdSerial.write(LCD_CMD_LISTEN);
    lcdSerial.write(location);
    //                 12345678901234567890
    lcdSerial.print(F("                    "));
}

void lcdPrint(byte location, const __FlashStringHelper* string) {
    lcdSerial.write(LCD_CMD_LISTEN);
    lcdSerial.write(location);
    lcdSerial.print(string);
}

void lcdPrint(byte location, String string) {
    lcdSerial.write(LCD_CMD_LISTEN);
    lcdSerial.write(location);
    lcdSerial.print(string);
}

void lcdPrint(byte location, int string) {
    lcdSerial.write(LCD_CMD_LISTEN);
    lcdSerial.write(location);
    lcdSerial.print(string);
}

void lcdPrintLine(byte location, const __FlashStringHelper* string) {
    lcdClearLine(location);
    lcdPrint(location, string);
}
