#define N_ROWS 4
#define N_COLS 4

#define UP     '<'
#define DOWN   '>'
#define CLEAR  '_'

#define SHIFT  '^'
#define HELP   '?'
#define ENTER  '!'

char keys[N_ROWS][N_COLS] = { // define keys for keypad, i.e. Keymap
    { '1',   '2', '3',  UP    },
    { '4',   '5', '6',  DOWN  },
    { '7',   '8', '9',  SHIFT },
    { CLEAR, '0', HELP, ENTER }
};

byte rowPins[N_ROWS] = { 35, 37, 39, 41 }; // C0 C1 C2 C3 to Mega pins
byte colPins[N_COLS] = { 43, 45, 47, 49 }; // R0 R1 R2 R3 to Mega pins

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, N_ROWS, N_COLS);

// -------------------------------------------------------------------

char keypadGetKey() {
  return keypad.getKey();
}
