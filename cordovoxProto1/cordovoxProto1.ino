

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

volatile uint8_t chip1a = 0xff, 
        chip1b = 0xff, 
        chip2a = 0xff, 
        chip2b = 0xff, 
        chip3a = 0xff, 
        chip3b = 0xff, 
        chip4a = 0xff, 
        chip4b = 0xff,
        chip5a = 0xff, 
        chip5b = 0xff ; // key states. pin is pulled low when key is pressed

// a map from chip/pin to midi notes. this is meant for initial debugging, but will be deprecieated for more abstraction/control of note assignment
//                                     a0   a1    a2   a3   a4   a5     a6   a7  b0    b1    b2    b3   b4   b5    b6    b7
const byte   chip1_midi_notes[16] = { NE4, NF3,  NB2, NF2, NDS5, NA4, NE5,  NC3, NG2, NAS3, NFS4, NGS2, NCS4, NFS3, NB3, NG5 };
const byte   chip2_midi_notes[16] = { NDS4, NFS3, NF5,NG3, NGS4, NCS5, ND5, NF4, NFS5, NGS3, ND3, NB4, NC4, NAS2, NG4, NE3 };
const byte   chip3_midi_notes[16] = { NFS1, NAS3, NB3, NFS1, NCS4, NB1, NE1, NA2, NFS2, NAS4, NDS3, NCS3, NA4, ND4, NA3, NC5 };
const byte   chip4_midi_notes[16] = { NA3, NC1, NB3, NC4, NCS4, NCS1, NCS1, NE4,  NAS1, ND1, NG1, NF1, NGS1, NA1, NDS1, NB1 };
const byte   chip5_midi_notes[16] = { NA3, NGS1, NB3, NC4, NCS4, ND4, NDS4, NE4, NF4, NFS4, NG4, NGS4, NA4, NAS4, NB4, NC4 };

// pin to key map. Left hand bass c = 0, Left hand chord c = 12. Right hand low f = 24. 99 means 'unassigned'
//                                 a0   a1    a2   a3   a4   a5   a6   a7  b0   b1  b2   b3   b4   b5    b6  b7
const uint8_t key_num_chip1[16] = { 47,  36,   30,  24,  57,  52,  59,  31, 26,  41, 49,  27,  44,  37,  42,  61 };
const uint8_t key_num_chip2[16] = {   46,  37,   60,  38,  51,  56,  57,  47, 61,  39, 33,  54,  43,  29,  50,  35 };
const uint8_t key_num_chip3[16] = {   6,  22,   11,  6,  44,  11,  04,  28, 25,  53, 34,  32,  52,  45,  40,  55 };
const uint8_t key_num_chip4[16] = {   99,  0,   99,  43,  99,  29,  99,  47, 10,  2, 7,  5,  99,  9,  3,  11 };
const uint8_t key_num_chip5[16] = {   99,  8,   99,  99,  99,  99,  99,  99, 99,  99, 99,  99,  99,  99,  99,  99 };

// map a key number to a bank. 0 - left hand bass, 1 - left hand chord, 2 - right hand
#define WHICH_BANK(s)  (((s) < 12) ? 0 : (((s) > 11) && (s < 24)) ? 1 : 2)

uint8_t bank0_midi_chan = 0;
uint8_t bank1_midi_chan = 0;
uint8_t bank2_midi_chan = 0;

// each bank has an octave assignment. 0 is 'base/normal'. add 12 to raise the octave, subtract 12 to lower it.
// the octave change is applied to the midi note numbers.
uint8_t bank0_octave = 0;
uint8_t bank1_octave = 0;
uint8_t bank2_octave = 0;
uint8_t is_bank_enabled[4] = { 1, 0, 1};  // indexed by bank number

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("hello");

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

  setup_23s17s(CHIP1_CS);
  setup_23s17s(CHIP2_CS);
  setup_23s17s(CHIP3_CS);
  setup_23s17s(CHIP4_CS);
  setup_23s17s(CHIP5_CS);

}

void loop() {
  delay(5);
  process_chip(CHIP1_CS, &chip1a, &chip1b, chip1_midi_notes,  1);
  process_chip(CHIP2_CS, &chip2a, &chip2b, chip2_midi_notes,  2);
  process_chip(CHIP3_CS, &chip3a, &chip3b, chip3_midi_notes,  3);
  process_chip(CHIP4_CS, &chip4a, &chip4b, chip4_midi_notes,  4);
  process_chip(CHIP5_CS, &chip5a, &chip5b, chip5_midi_notes,   5);

//  read_control_chip();

 
}

/*
void read_chip(unsigned int chip_cs, volatile uint8_t *chip_statea, volatile uint8_t *chip_stateb, const byte *note_map, char* which) {
  uint8_t pin_states;
  uint8_t pinsa, pinsb;

  digitalWrite(chip_cs, LOW);
  SPI.transfer(OPCODE_READ >> 8);
  SPI.transfer(GPIOA);
  pinsa = SPI.transfer(0);
  pinsb = SPI.transfer(0);
  digitalWrite(chip_cs, HIGH);

  pin_states = pinsa;
  
  if (pin_states != *chip_statea) {
     Serial.print(which); Serial.print("a:"); Serial.print(*chip_statea,HEX); Serial.print(" ");
     Serial.println(pin_states, HEX);
     
  if ((pin_states & 0x1) != (*chip_statea & 0x1)) {
    if (pin_states & 0x1) MIDI.sendNoteOff(note_map[0], 0, 1); else MIDI.sendNoteOn(note_map[0], VEL, 1);
    }
    if ((pin_states & 0x2) != (*chip_statea & 0x2)) {
      if (pin_states & 0x2) MIDI.sendNoteOff(note_map[1], 0, 1); else MIDI.sendNoteOn(note_map[1], VEL, 1);
    }
    if ((pin_states & 0x4) != (*chip_statea & 0x4)) {
      if (pin_states & 0x4) MIDI.sendNoteOff(note_map[2], 0, 1); else MIDI.sendNoteOn(note_map[2], VEL, 1);
    }
    if ((pin_states & 0x8) != (*chip_statea & 0x8)) {
      if (pin_states & 0x8) MIDI.sendNoteOff(note_map[3], 0, 1); else MIDI.sendNoteOn(note_map[3], VEL, 1);
    }
    if ((pin_states & 0x10) != (*chip_statea & 0x10)) {
      if (pin_states & 0x10) MIDI.sendNoteOff(note_map[4], 0, 1); else MIDI.sendNoteOn(note_map[4], VEL, 1);
    }
    if ((pin_states & 0x20) != (*chip_statea & 0x20)) {
      if (pin_states & 0x20) MIDI.sendNoteOff(note_map[5], 0, 1); else MIDI.sendNoteOn(note_map[5], VEL, 1);
    }
    if ((pin_states & 0x40) != (*chip_statea & 0x40)) {
      if (pin_states & 0x40) MIDI.sendNoteOff(note_map[6], 0, 1); else MIDI.sendNoteOn(note_map[6], VEL, 1);
    }
    if ((pin_states & 0x80) != (*chip_statea & 0x80)) {
      if (pin_states & 0x80) MIDI.sendNoteOff(note_map[7], 0, 1); else MIDI.sendNoteOn(note_map[7], VEL, 1);
    }
   *chip_statea = pin_states;  // save the changed pin states
  }


  pin_states = pinsb;
  if (pin_states != *chip_stateb) {

    Serial.print(which); Serial.print("b:");
    Serial.println(pin_states, HEX);
    if ((pin_states & 0x1) != (*chip_stateb & 0x1)) {
//      Serial.println("cp1 pin1 change");
    if (pin_states & 0x1) MIDI.sendNoteOff(note_map[8], 0, 1); else MIDI.sendNoteOn(note_map[8], VEL, 1);
    }
    if ((pin_states & 0x2) != (*chip_stateb & 0x2)) {
      if (pin_states & 0x2) MIDI.sendNoteOff(note_map[9], 0, 1); else MIDI.sendNoteOn(note_map[9], VEL, 1);
    }
    if ((pin_states & 0x4) != (*chip_stateb & 0x4)) {
      if (pin_states & 0x4) MIDI.sendNoteOff(note_map[10], 0, 1); else MIDI.sendNoteOn(note_map[10], VEL, 1);
    }
    if ((pin_states & 0x8) != (*chip_stateb & 0x8)) {
      if (pin_states & 0x8) MIDI.sendNoteOff(note_map[11], 0, 1); else MIDI.sendNoteOn(note_map[11], VEL, 1);
    }
    if ((pin_states & 0x10) != (*chip_stateb & 0x10)) {
      if (pin_states & 0x10) MIDI.sendNoteOff(note_map[12], 0, 1); else MIDI.sendNoteOn(note_map[12], VEL, 1);
    }
    if ((pin_states & 0x20) != (*chip_stateb & 0x20)) {
      if (pin_states & 0x20) MIDI.sendNoteOff(note_map[13], 0, 1); else MIDI.sendNoteOn(note_map[13], VEL, 1);
    }
    if ((pin_states & 0x40) != (*chip_stateb & 0x40)) {
      if (pin_states & 0x40) MIDI.sendNoteOff(note_map[14], 0, 1); else MIDI.sendNoteOn(note_map[14], VEL, 1);
    }
    if ((pin_states & 0x80) != (*chip_stateb & 0x80)) {
      if (pin_states & 0x80) MIDI.sendNoteOff(note_map[15], 0, 1); else MIDI.sendNoteOn(note_map[15], VEL, 1);
    }
   *chip_stateb = pin_states;  // save the changed pin states
  }
}
*/

void process_chip(unsigned int chip_cs, volatile uint8_t *chip_statea, volatile uint8_t *chip_stateb, const byte *note_map, uint8_t which) {
  uint8_t pin_states;
  uint8_t pinsa, pinsb;
  const uint8_t *pkey_num_map;

  digitalWrite(chip_cs, LOW);
  SPI.transfer(OPCODE_READ >> 8);
  SPI.transfer(GPIOA);
  pinsa = SPI.transfer(0);
  pinsb = SPI.transfer(0);
  digitalWrite(chip_cs, HIGH);

  pin_states = pinsa;
  switch(which) {
    case 1: 
      pkey_num_map = key_num_chip1;
      break;
    case 2:
      pkey_num_map = key_num_chip2;
      break;
    case 3:
      pkey_num_map = key_num_chip3;
      break;
    case 4:
      pkey_num_map = key_num_chip4;
      break;
    case 5:
      pkey_num_map = key_num_chip5;
      break;
    default:
      Serial.print("bad key_num_chip value");
      
  }
  
  if (pin_states != *chip_statea) {
     Serial.print(which,HEX); Serial.print("a:"); Serial.print(*chip_statea,HEX); Serial.print(" ");
     Serial.print(pin_states, HEX); Serial.print(" ");
     
    if ((pin_states & 0x1) != (*chip_statea & 0x1) && (pkey_num_map[0] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[0])]) ) {
      if (pin_states & 0x1) MIDI.sendNoteOff(note_map[0], 0, 1); else MIDI.sendNoteOn(note_map[0], VEL, 1);
      Serial.println(pkey_num_map[0]);
    }
    if ((pin_states & 0x2) != (*chip_statea & 0x2) && (pkey_num_map[1] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[1])]) ) {
      if (pin_states & 0x2) MIDI.sendNoteOff(note_map[1], 0, 1); else MIDI.sendNoteOn(note_map[1], VEL, 1);
      Serial.println(pkey_num_map[1]);
    }
    if ((pin_states & 0x4) != (*chip_statea & 0x4) && (pkey_num_map[2] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[2])]) ) {
      if (pin_states & 0x4) MIDI.sendNoteOff(note_map[2], 0, 1); else MIDI.sendNoteOn(note_map[2], VEL, 1);
      Serial.println(pkey_num_map[2]);
    }
    if ((pin_states & 0x8) != (*chip_statea & 0x8) && (pkey_num_map[3] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[3])]) ) {
      if (pin_states & 0x8) MIDI.sendNoteOff(note_map[3], 0, 1); else MIDI.sendNoteOn(note_map[3], VEL, 1);
      Serial.println(pkey_num_map[3]);
    }
    if ((pin_states & 0x10) != (*chip_statea & 0x10) && (pkey_num_map[4] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[4])]) ) {
      if (pin_states & 0x10) MIDI.sendNoteOff(note_map[4], 0, 1); else MIDI.sendNoteOn(note_map[4], VEL, 1);
      Serial.println(pkey_num_map[4]);
    }
    if ((pin_states & 0x20) != (*chip_statea & 0x20) && (pkey_num_map[5] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[5])]) ) {
      if (pin_states & 0x20) MIDI.sendNoteOff(note_map[5], 0, 1); else MIDI.sendNoteOn(note_map[5], VEL, 1);
      Serial.println(pkey_num_map[5]);
    }
    if ((pin_states & 0x40) != (*chip_statea & 0x40) && (pkey_num_map[6] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[6])]) ) {
      if (pin_states & 0x40) MIDI.sendNoteOff(note_map[6], 0, 1); else MIDI.sendNoteOn(note_map[6], VEL, 1);
      Serial.println(pkey_num_map[6]);
    }
    if ((pin_states & 0x80) != (*chip_statea & 0x80) && (pkey_num_map[7] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[7])]) ) {
      if (pin_states & 0x80) MIDI.sendNoteOff(note_map[7], 0, 1); else MIDI.sendNoteOn(note_map[7], VEL, 1);
      Serial.println(pkey_num_map[7]);
    }
   *chip_statea = pin_states;  // save the changed pin states
  }


  pin_states = pinsb;
  if (pin_states != *chip_stateb) {

    Serial.print(which,HEX); Serial.print("b:");
    Serial.print(pin_states, HEX); Serial.print(" ");
    if ((pin_states & 0x1) != (*chip_stateb & 0x1) && (pkey_num_map[8] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[8])]) ) {
//      Serial.println("cp1 pin1 change");
      if (pin_states & 0x1) MIDI.sendNoteOff(note_map[8], 0, 1); else MIDI.sendNoteOn(note_map[8], VEL, 1);
      Serial.println(pkey_num_map[8]);
    }
    if ((pin_states & 0x2) != (*chip_stateb & 0x2) && (pkey_num_map[9] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[9])]) ) {
      if (pin_states & 0x2) MIDI.sendNoteOff(note_map[9], 0, 1); else MIDI.sendNoteOn(note_map[9], VEL, 1);
      Serial.println(pkey_num_map[9]);
    }
    if ((pin_states & 0x4) != (*chip_stateb & 0x4) && (pkey_num_map[10] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[10])]) ) {
      if (pin_states & 0x4) MIDI.sendNoteOff(note_map[10], 0, 1); else MIDI.sendNoteOn(note_map[10], VEL, 1);
      Serial.println(pkey_num_map[00]);
    }
    if ((pin_states & 0x8) != (*chip_stateb & 0x8) && (pkey_num_map[11] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[11])]) ) {
      if (pin_states & 0x8) MIDI.sendNoteOff(note_map[11], 0, 1); else MIDI.sendNoteOn(note_map[11], VEL, 1);
      Serial.println(pkey_num_map[11]);
    }
    if ((pin_states & 0x10) != (*chip_stateb & 0x10) && (pkey_num_map[12] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[12])]) ) {
      if (pin_states & 0x10) MIDI.sendNoteOff(note_map[12], 0, 1); else MIDI.sendNoteOn(note_map[12], VEL, 1);
      Serial.println(pkey_num_map[12]);
    }
    if ((pin_states & 0x20) != (*chip_stateb & 0x20) && (pkey_num_map[13] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[13])]) ) {
      if (pin_states & 0x20) MIDI.sendNoteOff(note_map[13], 0, 1); else MIDI.sendNoteOn(note_map[13], VEL, 1);
      Serial.println(pkey_num_map[13]);
    }
    if ((pin_states & 0x40) != (*chip_stateb & 0x40) && (pkey_num_map[14] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[14])]) ) {
      if (pin_states & 0x40) MIDI.sendNoteOff(note_map[14], 0, 1); else MIDI.sendNoteOn(note_map[14], VEL, 1);
      Serial.println(pkey_num_map[14]);
    }
    if ((pin_states & 0x80) != (*chip_stateb & 0x80) && (pkey_num_map[15] != 99) && (is_bank_enabled[WHICH_BANK(pkey_num_map[15])]) ) {
      if (pin_states & 0x80) MIDI.sendNoteOff(note_map[15], 0, 1); else MIDI.sendNoteOn(note_map[15], VEL, 1);
      Serial.println(pkey_num_map[15]);
    }
   *chip_stateb = pin_states;  // save the changed pin states
  }
}

//void send_note(int opcode, uint8_t note, int velocity) {
//
//  Serial.write(opcode); Serial.write(note); Serial.write(velocity);
//}

void setup_23s17s(unsigned int chip) {
  uint8_t register_v;
  
  SPI.begin();
//  SPI.beginTransaction(SPISettings(10000, MSBFIRST, SPI_MODE0)); // initialize SPI interface


  digitalWrite(chip, LOW);
  SPI.transfer(OPCODE_READ >> 8); SPI.transfer(GPPUA_ADDR); register_v = SPI.transfer(0);
  digitalWrite(chip, HIGH);
  Serial.println(register_v, HEX);
//  delay(2000);

  digitalWrite(chip, LOW);
//  SPI.transfer16(OPCODE_WRITE | GPPUA_ADDR);
  SPI.transfer(OPCODE_WRITE >> 8);
  SPI.transfer(GPPUA_ADDR);
  SPI.transfer(GPPU_VALUE);
  SPI.transfer(GPPU_VALUE);
  digitalWrite(CHIP1_CS, HIGH);

  digitalWrite(chip, LOW);
  SPI.transfer(OPCODE_READ >> 8); SPI.transfer(GPPUA_ADDR); register_v = SPI.transfer(0);
  digitalWrite(chip, HIGH);
  Serial.println(register_v, HEX);
//  delay(2000);



}
