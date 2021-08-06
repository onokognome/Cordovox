

//#include <Adafruit_TinyUSB.h>



#include <MIDI.h>
#include <SPI.h>
#include "23s17.h"
#include "pin_key_mapping.h"

// Arduino to 23S17 Chip Select Mapping
#define CHIP1_CS  5
#define CHIP2_CS  6
#define CHIP3_CS  10
#define CHIP4_CS  11
#define CHIP5_CS  12

#define VEL       127    // midi key velocity



#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
  // Required for Serial on Zero based boards
  #define Serial SERIAL_PORT_USBVIRTUAL
#endif

MIDI_CREATE_DEFAULT_INSTANCE();

int notes[] = {69, 72, 74, 76, 72, 81, 79};  // melody notes
int vels[] = {127, 96, 64, 96, 32, 127, 64};  // velocity per note
int rests[] = {50, 50, 50, 50, 50, 200, 50};  // rests between notes
int note_mods[] = {0, 0, 0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 5, 5, 3, 3};  // modifies notes for progression

uint8_t chip1a = 0xff, 
        chip1b = 0xff, 
        chip2a = 0xff, 
        chip2b = 0xff, 
        chip3a = 0xff, 
        chip3b = 0xff, 
        chip4a = 0xff, 
        chip4b = 0xff ; // key states. pin is pulled low when key is pressed


void setup() {
  // put your setup code here, to run once:
 delay(4000);

  Serial.begin(11520);
  Serial.println("hello");
//  send_note(0x90, NF1, 127);
//  delay(1000);
//  send_note(0x90, NF1, 0);
  
  MIDI.begin(MIDI_CHANNEL_OMNI);

  pinMode(CHIP1_CS, OUTPUT);
  pinMode(CHIP2_CS, OUTPUT);
  pinMode(CHIP3_CS, OUTPUT);
  pinMode(CHIP4_CS, OUTPUT);
  pinMode(CHIP5_CS, OUTPUT);
  digitalWrite(CHIP1_CS, HIGH);
  digitalWrite(CHIP2_CS, HIGH);
  digitalWrite(CHIP3_CS, HIGH);
  digitalWrite(CHIP4_CS, HIGH);
  digitalWrite(CHIP5_CS, HIGH);

  MIDI.sendNoteOn(NC2, 127, 1);
  setup_23s17s();
 
 // for(int j=0; j<16; j++){  // loop through four measures for progression
 //   for(int i=0; i<7; i++){ //
 //     MIDI.sendNoteOn(notes[i]+note_mods[j], vels[i], 1);
 //     delay(100);
 //     MIDI.sendNoteOff(notes[i]+note_mods[j], 0, 1);
 //     delay(rests[i]);
//  }}

  MIDI.sendNoteOn(NC1, 127, 1);
}

void loop() {
  uint8_t pin_states;
  delay(100);
  digitalWrite(CHIP1_CS, LOW);
//  delay(1);
//  SPI.transfer(OPCODE_READ);
//  pin_states = SPI.transfer16(OPCODE_READ | GPIOA);
  SPI.transfer(OPCODE_READ >> 8);
  SPI.transfer(GPIOA);
  pin_states = SPI.transfer(0);
  digitalWrite(CHIP1_CS, HIGH);

  Serial.println(pin_states, HEX);
  
  // pin_states = 0xFC;
  if (pin_states != chip1a) {
    if ((pin_states & 0x1) != (chip1a & 0x1)) {
//      Serial.println("cp1 pin1 change");
    if (pin_states & 0x1) MIDI.sendNoteOff(CP1_A0, 0, 1); else MIDI.sendNoteOn(CP1_A0, VEL, 1);
    }
    if ((pin_states & 0x2) != (chip1a & 0x2)) {
      if (pin_states & 0x2) MIDI.sendNoteOff(CP1_A1, 0, 1); else MIDI.sendNoteOn(CP1_A1, VEL, 1);
    }
    if ((pin_states & 0x4) != (chip1a & 0x4)) {
      if (pin_states & 0x4) MIDI.sendNoteOff(CP1_A2, 0, 1); else MIDI.sendNoteOn(CP1_A2, VEL, 1);
    }
    if ((pin_states & 0x8) != (chip1a & 0x8)) {
      if (pin_states & 0x8) MIDI.sendNoteOff(CP1_A3, 0, 1); else MIDI.sendNoteOn(CP1_A3, VEL, 1);
    }
    if ((pin_states & 0x10) != (chip1a & 0x10)) {
      if (pin_states & 0x10) MIDI.sendNoteOff(CP1_A4, 0, 1); else MIDI.sendNoteOn(CP1_A4, VEL, 1);
    }
    if ((pin_states & 0x20) != (chip1a & 0x20)) {
      if (pin_states & 0x20) MIDI.sendNoteOff(CP1_A5, 0, 1); else MIDI.sendNoteOn(CP1_A5, VEL, 1);
    }
    if ((pin_states & 0x40) != (chip1a & 0x40)) {
      if (pin_states & 0x40) MIDI.sendNoteOff(CP1_A6, 0, 1); else MIDI.sendNoteOn(CP1_A6, VEL, 1);
    }
    if ((pin_states & 0x80) != (chip1a & 0x80)) {
      if (pin_states & 0x80) MIDI.sendNoteOff(CP1_A7, 0, 1); else MIDI.sendNoteOn(CP1_A7, VEL, 1);
    }
   chip1a = pin_states;  // save the changed pin states
  }

}


void send_note(int opcode, int note, int velocity) {
  Serial.write(opcode); Serial.write(note); Serial.write(velocity);
}

void setup_23s17s() {
  SPI.begin();
  SPI.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0)); // initialize SPI interface

//  digitalWrite(CHIP1_CS, LOW);
//  SPI.transfer16(OPCODE_WRITE | IOCON_ADDR);
//  SPI.transfer(IOCON_VALUE);
//  digitalWrite(CHIP1_CS, HIGH);

  digitalWrite(CHIP1_CS, LOW);
//  SPI.transfer16(OPCODE_WRITE | GPPUA_ADDR);
  SPI.transfer(OPCODE_WRITE >> 8);
  SPI.transfer(GPPUA_ADDR);
  SPI.transfer(GPPU_VALUE);
  SPI.transfer(GPPU_VALUE);
  digitalWrite(CHIP1_CS, HIGH);

//  digitalWrite(CHIP1_CS, LOW);
//  SPI.transfer16(OPCODE_WRITE | GPPUB_ADDR);
//  SPI.transfer(GPPU_VALUE);
//  digitalWrite(CHIP1_CS, HIGH);

}
