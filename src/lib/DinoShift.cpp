//
// This file adds to the Dino class only if DINO_SHIFT is defined in Dino.h.
//
#include "Dino.h"
#ifdef DINO_SHIFT

// Define listeners for ShiftIn registers.
#define SHIFT_LISTENER_COUNT 4
struct shiftListener{
  byte     latchPin;
  byte     len;
  byte     dataPin;
  byte     clockPin;
  byte     clockHighFirst;
  boolean  enabled;
};
shiftListener shiftListeners[SHIFT_LISTENER_COUNT];


//
// Request format for shift register API functions.
// pin        = latch pin (int)
// val        = length (int)
// auxMsg[0]  = data pin (byte)
// auxMsg[1]  = clock pin (byte)
// auxMsg[2]  = send clock high before reading (byte) (0/1) (read func only)
// auxMsg[3]+ = data (bytes) (write func only)
//
// CMD = 21
// Write to a shift register.
void Dino::shiftWrite(int latchPin, int len, byte dataPin, byte clockPin, byte *data) {
  // Set latch pin low to begin serial write.
  digitalWrite(latchPin, LOW);

  // Write one byte at a time.
  for (uint8_t i = 0;  i < len;  i++) {
    shiftOut(dataPin, clockPin, LSBFIRST, data[i]);
  }

  // Set latch pin high so register writes to parallel output.
  digitalWrite(latchPin, HIGH);
}


// CMD = 22
// Read from a shift register.
void Dino::shiftRead(int latchPin, int len, byte dataPin, byte clockPin, byte clockHighFirst) {
  // Send clock pin high if using a register that clocks on rising edges.
  // If not, the MSB will not be read on those registers (always 1),
  // and all other bits will be shifted by 1 towards the LSB.
  if (clockHighFirst > 0) digitalWrite(clockPin, HIGH);

  // Latch high to read parallel state, then low again to stop.
  digitalWrite(latchPin, HIGH);
  digitalWrite(latchPin, LOW);

  // Send data as if coming from the latch pin so it's easy to identify.
  // Start with just pin number and : for now.
  sprintf(response, "%d:", latchPin);
  _writeCallback(response);

  for (int i = 1;  i <= len;  i++) {
    // Read a single byte from the register.
    byte reading = shiftIn(dataPin, clockPin, LSBFIRST);

    // If we're on the last byte, append \n. If not, append a comma, then write.
    if (i == len) {
      sprintf(response, "%d\n", reading);
    } else {
      sprintf(response, "%d,", reading);
    }
    _writeCallback(response);
  }

  // Leave latch pin high and clear response so main loop doesn't send anything.
  digitalWrite(latchPin, HIGH);
  response[0] = '\0';
}


// CMD = 23
// Start listening to a register using the Arduino shiftIn function.
// Overwrite the first disabled listener in the struct array.
void Dino::addShiftListener(int latchPin, int len, byte dataPin, byte clockPin, byte clockHighFirst) {
  for (int i = 0;  i < SHIFT_LISTENER_COUNT;  i++) {
    if (shiftListeners[i].enabled == false) {
      shiftListeners[i] = {
        latchPin,
        len,
        dataPin,
        clockPin,
        clockHighFirst,
        true
      };
      return;
    } else {
    // Should send some kind of error if all are in use.
    }
  }
}


// CMD = 24
// Send a number for a latch pin to remove a shift register listener.
void Dino::removeShiftListener() {
  for (int i = 0;  i < SHIFT_LISTENER_COUNT;  i++) {
    if (shiftListeners[i].latchPin == pin) {
      shiftListeners[i].enabled = false;
    }
  }
}


// Gets called by Dino::updateListeners to run listeners in the main loop.
void Dino::updateShiftListeners() {
  for (int i = 0; i < SHIFT_LISTENER_COUNT; i++) {
    if (shiftListeners[i].enabled) {
      shiftRead(shiftListeners[i].latchPin,
                shiftListeners[i].len,
                shiftListeners[i].dataPin,
                shiftListeners[i].clockPin,
                shiftListeners[i].clockHighFirst);
    }
  }
}


// Gets called by Dino::reset to clear all listeners.
void Dino::clearShiftListeners() {
  for (int i = 0; i < SHIFT_LISTENER_COUNT; i++) shiftListeners[i].enabled = false;
}

#endif