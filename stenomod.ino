/*
Copyright (C) 2016 by Jason Green.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License fo details.
e Free Softwarer more
Foundation, Inc., 59 Temple Place, Suite 330, Boston
You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to th, MA 02111-1307  USA

For LGPL information:   http://www.gnu.org/copyleft/lesser.txt
*/

/*
Adapted to the gemini protocol by @percidae
Adapted to a Palantype machine by @percidae
Converts a hardwarematrix of 5x6 to the Palantype Gemini Protocol

Adapted to a Palantype machine with two additional keys by @percidae

 */


#define REPEAT_ENABLED true
#define REPEAT_TIMEOUT  300
#define REPEAT_START_DELAY 800
#define REPEAT_DELAY 45
#define STICKY_ENABLED true
#define STICKY_DELAY 500

// Keyboard matrix columns
#define KB_COLUMNS 5   // 7 for gemini, 6 for tx bolt, 6 for Palantype with gemini protocol, 5 for Palantype De with gemini protocol
// Keyboard matrix rows
#define KB_ROWS 8      // 6 for gemini, 4 for tx bolt, 5 for Palantype with gemini protocol, 8 for Palantype De with gemini protocol

// Useful way to refer to the Stroke objects
typedef uint8_t* Stroke;

typedef enum { NONE, REPEAT, STICKY, STICKY_SEND, NO_MODE } Mode;

typedef struct {
   Mode mode;
   uint8_t current_stroke[KB_ROWS];
   uint8_t current_keys[KB_ROWS];
   uint8_t current_keys_debounce[KB_ROWS];
   uint32_t debounce_time;
   uint8_t previous_stroke[KB_ROWS];
   uint8_t sticky_stroke[KB_ROWS];
   uint8_t last_stroke[KB_ROWS];
   uint8_t last_keys[KB_ROWS];
   uint32_t last_stroke_change;
   uint32_t last_key_change;
   uint32_t last_stroke_send;
   uint32_t last_key_up;
   bool stroke_sent;

   bool repeat_enabled;
   bool sticky_enabled;
   uint16_t repeat_timeout;
   uint16_t repeat_start_delay;
   uint16_t repeat_delay;
   uint16_t sticky_delay;
} State;

/*
 Gemini Keychart (not for Palantype or Palantype De)
  
                     5      7     8     9    10     6    11   (connected before the diodes)
 STENO_KEY_CHART = ("Fn", "#1", "#2", "#3", "#4", "#5", "#6",          14 (connected behind the diodes)
                   "S1-", "S2-", "T-", "K-", "P-", "W-", "H-",         15
                   "R-", "A-", "O-", "*1", "*2", "res1", "res2",       16
                   "pwr", "*3", "*4", "-E", "-U", "-F", "-R",          17
                   "-P", "-B", "-L", "-G", "-T", "-S", "-D",           18
                   "#7", "#8", "#9", "#A", "#B", "#C", "-Z")           19
 */

/*
 Palantype specific things 
 */
#define BIT8 B10000000
#define BIT7 B01000000
#define BIT6 B00100000
#define BIT5 B00010000
#define BIT4 B00001000
#define BIT3 B00000100
#define BIT2 B00000010
#define BIT1 B00000001

// Convert the read strokes to the Palantype Protocol
typedef struct {
   uint8_t read_byte;
   uint8_t read_bit;
   uint8_t gemini_byte;
   uint8_t gemini_bit;  
} Palantype_Conversion_Matrix;

/* vmtl immer: 0BIT6, 0BIT7, 1BIT7, 1BIT6 2BIT6 2BIT7 3BIT7 3BIT6 4BIT6 4BIT7 5BIT7 5BIT6 6BIT6 6BIT7 7BIT7 7BIT6*/
/* ?: 0 BIT8 1BIT8 2BIT1 2BIT3 2BIT4 2BIT5 2BIT8 3BIT8 3BIT5 3BIT4 3BIT3 3BIT1 4BIT8 5BIT8 6BIT8 7BIT8 */
/* linke special Taste: 2BIT2 output 3BIT6, */
/* rechte special Taste: 3BIT output 3BIT5 */
Palantype_Conversion_Matrix conversion[35] = {
   {0, BIT2, 1, BIT7}, // C-
   {1, BIT2, 1, BIT6}, // S- 
   {0, BIT5, 0, BIT4}, // P-
   {1, BIT5, 1, BIT5}, // T-
   {6, BIT5, 1, BIT4}, // H-
   {0, BIT4, 2, BIT6}, // M-
   {1, BIT4, 1, BIT3}, // F-
   {6, BIT4, 1, BIT2}, // R-
   {0, BIT3, 0, BIT2}, // N-
   {1, BIT3, 1, BIT1}, // L-
   {6, BIT3, 2, BIT7}, // Y-
   {0, BIT1, 2, BIT5}, // O-
   {6, BIT1, 2, BIT3}, // E-
   {1, BIT1, 2, BIT4}, // I- (left)
   {6, BIT2, 0, BIT7}, // +-
   {6, BIT2, 0, BIT3}, // +-
   {6, BIT2, 2, BIT1}, // +-
   {7, BIT3, 3, BIT1}, // -N
   {4, BIT3, 3, BIT2}, // -L
   {5, BIT3, 5, BIT6}, // -C
   {7, BIT4, 3, BIT3}, // -M
   {4, BIT4, 4, BIT7}, // -F
   {5, BIT4, 5, BIT5}, // -R
   {7, BIT5, 4, BIT4}, // -P
   {4, BIT5, 4, BIT5}, // -T
   {5, BIT5, 5, BIT4}, // -+
   {7, BIT2, 4, BIT3}, // -H
   {4, BIT2, 5, BIT3}, // -S
  // {4, BIT1, 2, BIT4}, // -I (right)
   {4, BIT1, 2, BIT4}, // -I (right) as Asterisk!
   {7, BIT1, 3, BIT4}, // -A
   {5, BIT1, 5, BIT2}, // -U
   {5, BIT2, 4, BIT6}, // -^
   {5, BIT2, 4, BIT1}, // -^
   {2, BIT2, 3, BIT5}, // *3 for nospace, left extra key 3BIT6
   {3, BIT2, 3, BIT6}  // *4 for capitalization, right extra key 3BIT5
  };

// Using Arduino pin numbers
uint8_t inpin[KB_COLUMNS] = {5, 6, 7, 8, 9}; // 6 columns for Palantype with gemini protocol, 5 for Palantype De
uint8_t pin[KB_ROWS] = {4, 3, 2, 16, 14, 15, 21, 10}; // 6 rows for Palantype with gemini protocol, 8 for Palantype De
uint8_t LED = 13;
uint64_t last_key_up = 0;

// Special to convert the read stroke to a gemini stroke
uint8_t gemini_stroke[6] = {B10000000, 0, 0, 0, 0, 0};

State state;

void default_settings();
void reset_state(bool set_prev);

// Setup ports and serial
// All unused ports get weak pullup
void setup() {
  for (int i = 0; i < 22; i++) {
    pinMode (i, INPUT_PULLUP);
  }
  for (int i = 0; i < KB_ROWS; i++) {
    pinMode (pin [i], INPUT);
  }
  led(false);
  Serial.begin(9600);
  reset_state(false);
  default_settings();
}

void default_settings() {
  state.repeat_enabled = REPEAT_ENABLED;
  state.sticky_enabled = STICKY_ENABLED;
  state.repeat_timeout = REPEAT_TIMEOUT;
  state.repeat_start_delay = REPEAT_START_DELAY;
  state.repeat_delay = REPEAT_DELAY;
  state.sticky_delay = STICKY_DELAY;
}

void printBits(byte myByte){
 for(byte mask = 0x80; mask; mask >>= 1){
   if(mask  & myByte)
       Serial.print('1');
   else
       Serial.print('0');
 }
}

/*** Stroke Manipulation Functions */
// Reset a stroke to 0
Stroke clear_stroke(Stroke s) {
   return memset(s, 0, KB_ROWS);
}

// Copy one stroke to another
Stroke copy_stroke(Stroke d, Stroke s) {
   return memcpy(d, s, KB_ROWS);
}

// Compare two stokes
Stroke compare_stroke(Stroke a, Stroke b) {
   return memcmp(a, b, KB_ROWS);
}

// Merge two strokes
Stroke merge_stroke(Stroke d, Stroke s) {
   for (int i = 0; i < KB_ROWS; i++)
     d[i] = d[i] | s[i];
   return d;
}

/* Hardware Functions */
// Turn LED on are or off
void led(bool on) {
   digitalWrite(LED, on ? HIGH : LOW);
}

// Set to output mode, and pull low
// to measure a row.
void set_output(uint8_t p) {
   pinMode(pin[p], OUTPUT);
   digitalWrite(pin[p], LOW);
}

// Set to input mode
void set_input(uint8_t p) {
   pinMode(pin[p], INPUT);
}

// Read the current byte, where
// bit value of 1 means key is
// pressed
uint8_t read_byte() {
   uint8_t x = 0;
   for (int i = 0; i < KB_COLUMNS; i++) {
      x *= 2;
      x |= digitalRead (inpin[i]);
   }
   return (x ^ 0x7f);
}

// Send byte over serial
void send_byte(uint8_t b) {
   Serial.write(b);
}

// Read a column of keys
uint8_t read_column(uint8_t p) {
   uint8_t ret;
   set_output(p);
   delayMicroseconds(10);
   ret = read_byte();
   set_input(p);
   return ret;
}

/* State Manipulation Functions */
// Resets state to "as new" settings.
// optinally sets previous_stroke to
// current_stroke
void reset_state(bool set_prev) {
   if (set_prev)
     copy_stroke(state.previous_stroke, state.current_stroke);
   else
     clear_stroke(state.previous_stroke);
   clear_stroke(state.current_stroke);
   clear_stroke(state.last_stroke);
   clear_stroke(state.last_keys);
   state.stroke_sent = false;
   state.mode = NONE;
}

/* Key Scanning Functions */
// Check all keys
// Stroke s is set to currently pressed keys,
//   overwriting existing value
// Stroke c is accumulative, or'ing with
//   existing value
// Return true if any key is currently pressed.

bool look(Stroke s, Stroke c) {
   bool ret = false;
   for (int i = 0; i < KB_ROWS; i++) {
     uint8_t r = read_column(i);
     ret |= r;
     if (c)
       c[i] |= r;
     if (s)
       s[i] = r;
   }
   return ret;
}

typedef struct {
  void (*func)();
  bool enabled;
  uint32_t time_set;
  uint32_t length;
} Timer;

void m_repeat_send();
void m_sticky_start();

Timer timers[] = {
    { m_repeat_send, false, 0 },
    { m_sticky_start, false, 0 }
};
#define M_REPEAT_SEND 0
#define M_STICKY_START 1
#define NUM_TIMERS 2

void set_timer(uint8_t t, uint32_t length) {
  timers[t].enabled = true;
  timers[t].length = length;
  timers[t].time_set = millis();
}

void unset_timer(uint8_t t) {
  timers[t].enabled = false;
}

void check_timers() {
  uint32_t now = millis();
  for (int i = 0; i < NUM_TIMERS; i++) {
    if (timers[i].enabled &&
         now - timers[i].time_set > timers[i].length) {
      timers[i].enabled = false;
      timers[i].func();
    }
  }
}

// Continuously poll for keys until
// Keys are pressed and then released
void scan_keys() {
   reset_state(true);

   // Wait until a key is pressed
   do {
     while (look(NULL, state.current_stroke) == false);
     // De-bounce
     delay(20);
   } while (look(NULL, state.current_stroke) == false);

   // Loop until all keys are lifted
   while (look(state.current_keys_debounce, state.current_stroke) == true) {
     merge_stroke(state.current_keys, state.current_keys_debounce);

     // Notify listeners of changes in keys that are currently pressed
     if (compare_stroke(state.current_keys, state.last_keys) != 0) {
       m_repeat_on_key_change();
       m_sticky_on_key_change();
     }

     // Notify listeners if the stroke has changed
     if (compare_stroke(state.current_stroke, state.last_stroke) != 0) {
       m_repeat_on_stroke_change();
     }

     // Check if any timers have run out
     check_timers();

     // Prepare for next iteration
     copy_stroke(state.last_keys, state.current_keys);
     copy_stroke(state.last_stroke, state.current_stroke);
     if (millis() - state.debounce_time > 20) {
       copy_stroke(state.current_keys, state.current_keys_debounce);
       state.debounce_time = millis();
     }
   }
   state.last_key_up = millis();

   unset_timer(M_REPEAT_SEND);
   unset_timer(M_STICKY_START);

   // If we haven't sent a key yet, send as normal
   if (state.stroke_sent == false)
     send_stroke(state.current_stroke);

   led(false);
}

/* Repeat Code */

void m_repeat_on_stroke_change() {
   if (state.repeat_enabled == false) return;

   /* Don't perform repeat if we're alreay in a mode */
   if (state.mode != NONE)
     return;

   /* Don't perform repeat if we've waited pass the timout */
   if (millis() - state.last_key_up > state.repeat_timeout)
     return;

   /* Don't perform repeat if the current stroke does not match previous
      stroke */
   if (compare_stroke(state.current_stroke, state.previous_stroke) != 0)
     return;

   /* Otherwise, we can start the repeat code */
   state.mode = REPEAT;

   /* Set a timer to initiate repeat after the appropriate delay */
   set_timer(M_REPEAT_SEND, state.repeat_start_delay);
}

void m_repeat_on_key_change() {
   /* Cancel repeat if key press change whilst repeat is on */
   if (state.mode == REPEAT) {
     state.mode = NO_MODE;
     unset_timer(M_REPEAT_SEND);
   }
}

void m_repeat_send() {
   if (state.mode != REPEAT) return;

   led(true);

   /* Send the current stroke */
   send_stroke(state.current_stroke);

   /* Add a timer to send the stroke again */
   set_timer(M_REPEAT_SEND, state.repeat_delay);
}


/* Sticky Code */

/* Called when sticky keys pressed for long enough */
void m_sticky_start() {
  if (state.mode != NONE) return;

  /* Save the key combination that is the base for the sticky keys */
  copy_stroke(state.sticky_stroke, state.current_stroke);

  /* Set mode to sticky */
  state.mode = STICKY;
  led(true);
}

void m_sticky_on_key_change() {
  if (state.sticky_enabled == false) return;

  if (state.mode == NONE) {
    /* Set a timer to initiate sicky mode */
    set_timer(M_STICKY_START, state.sticky_delay);
  }
  /* If additional keys have been pressed, track that we need to send */
  else if (state.mode == STICKY &&
         compare_stroke(state.sticky_stroke, state.current_stroke) != 0) {
    state.mode = STICKY_SEND;
  }
  /* once we get back to the initial set of keys, we can send the stroke */
  else if (state.mode == STICKY_SEND &&
         compare_stroke(state.sticky_stroke, state.current_keys) == 0) {
    send_stroke(state.current_stroke);
    copy_stroke(state.current_stroke, state.sticky_stroke);
    state.mode = STICKY;
  }
}

/* Serial Functions */
// Send the current stroke stored in
// b array
void send_stroke(uint8_t* b) {
   /*Serial.print(b[0]);
   Serial.print(b[1]);
   Serial.print(b[2]);
   Serial.print(b[3]);
   Serial.print(b[4]);
   Serial.print(b[5]);
   Serial.print(b[6]);
   Serial.print(b[7]);
   Serial.println();*/
   // Convert stroke to Palantype Gemini Protocol
   for (int i = 0; i < 35; i++){
    if((b[conversion[i].read_byte] & conversion[i].read_bit)== conversion[i].read_bit){
      gemini_stroke[conversion[i].gemini_byte] = gemini_stroke[conversion[i].gemini_byte] | conversion[i].gemini_bit;
      }
    }
   /*
   // Dirty Macro trick if the output would be an "I" but both I's were pressed to send ULFTS  is C P TH M, M FRNLYEIOI | A a | |for "delete last stroke" instead
   // Check if both I's were pressed and if so change the stroke
   if (gemini_stroke[0] == 0x80 and gemini_stroke[1] == 0x00 and gemini_stroke[3] == 0x00 and gemini_stroke[4] == 0x00 and gemini_stroke[5] == 0x00 and gemini_stroke[2] == BIT4){
      if((state.current_stroke[4] & 0x12) == 0x12){
        // Reset the I
        gemini_stroke[2] &= !BIT4;
        // Set to ULFTS
        gemini_stroke[5] |= BIT2;
        gemini_stroke[3] |= BIT2;
        gemini_stroke[4] |= BIT7;
        gemini_stroke[4] |= BIT5;
        gemini_stroke[5] |= BIT3;
      }
    }
   */
  
   // Always send all bytes even if empty
   for (int i = 0 ; i < 6; i++){
     send_byte(gemini_stroke[i]);
     // Delete after sending
     gemini_stroke[i] = 0x00;
   }
   // Set the first Bit in the first Gemini Byte to 1 for the next stroke
   gemini_stroke[0] |= B10000000;
   
   state.last_stroke_send = millis();
   state.stroke_sent = true;
   
}


/* Main Loop */
void loop() {
   while (true) {
     scan_keys();
   }
}
