#include <RK002.h>
RK002_DECLARE_INFO("Robinator", "Daniel Seipel, (dan@deseipel.com)", "0.5", "de45b790-49d4-42b4-b1e4-e196366f2e6d")
//
RK002_DECLARE_PARAM(NOTES_00, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_01, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_02, 1, 0, 127, 0)
//
RK002_DECLARE_PARAM(NOTES_03, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_04, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_05, 1, 0, 127, 0)
//
RK002_DECLARE_PARAM(NOTES_06, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_07, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_08, 1, 0, 127, 0)
//
RK002_DECLARE_PARAM(NOTES_09, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_10, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_11, 1, 0, 127, 0)
//
RK002_DECLARE_PARAM(NOTES_12, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_13, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_14, 1, 0, 127, 0)
//
RK002_DECLARE_PARAM(NOTES_15, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_16, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_17, 1, 0, 127, 0)
//
RK002_DECLARE_PARAM(NOTES_18, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_19, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_20, 1, 0, 127, 0)
//
RK002_DECLARE_PARAM(NOTES_21, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_22, 1, 0, 127, 0)
RK002_DECLARE_PARAM(NOTES_23, 1, 0, 127, 0)
//
// 24 notes to store. 24 banks.

//todo:  make it so the way you access Learn Chord Mode can be changed (with a note or something, not all MIDI keyboards may have STOP)
//todo:  measure the first instance of each channel's notes to get a baseline duration/gate.  If subsequent note offs fall short, try to
//    only send note off shen the duration is met.  This would stop note cut offs I think.


//reference
byte  Stop                  = 0xFC; ///< System Real Time - Stop
byte  ProgramChange         = 0xC9; ///< Program Change
byte  NoteOff               = 0x89; ///< Note Off
byte  NoteOn                = 0x99; ///< Note On
byte  Start                 = 0xFA; ///< System Real Time - Start
byte  Continue              = 0xFB; ///< System Real Time - Continue
byte  ControlChange         = 0xB9; ///< Control Change / Channel Mode

// for now, the method to access/exit LearnMode is the stop msg
byte LearnModeKey = Stop;
bool LearnMode_active = false;   //means the method by which you access learn Mode has not been activated.
byte chord_channel = 0;
byte note_channel = 1;
byte bass_channel = 2;
byte MelodyRobin_Note ;
byte RoundRobin_Chord[6];
byte OriGen_KeyMap[128][6];  //indexes 0-127 represent orginal note pressed, the values represent the generated note; sub indexes 0-127 represent the generated notes for that one key;
byte OriGen_ChordMap[128][6];  //indexes 0-127 represent original note pressed, the 6 values are the notes in the chord.  ;
byte OrigKey;
byte OrigKey_off;

volatile unsigned long ElapsedTime = 0;
volatile unsigned long StartTime = 0;
//count of LearnMode activate/deactivate msgs
byte LearnMode_count = 0;
byte bank = 0 ;

//turns features on/off
bool chords = true;
bool melody = true;
bool bass = true;

// 4 rows of 6 columns.  the row is the bank.
//[0-3 row][0-5 column]
const int row = 4;
const int column = 6;
int bank_matrix[row][column];
int w_total = 0;
int pool[30];
byte bass_note;
byte up_note = 0;
bool started = false;


bool bankset = true;

unsigned long PressBegin;
unsigned long PressEnd;
byte OnVelocity;

//  used for debugging
void Print_Vars()
{
  //RK002_printf("LearmModekey =%d", LearnModeKey);
  //RK002_printf("Bank %d", bank);
  //RK002_printf("row %d", row);
  //RK002_printf("column %d", column);
  //RK002_printf("bankset %d", bankset);
  //RK002_printf("PressBegin %d", PressBegin);
  //RK002_printf("PressEnd %d", PressEnd);
  //RK002_printf("OnVelocity %d", OnVelocity);
  //RK002_printf("LearnMode_count %d", LearnMode_count);
}


void Refresh_BankMatrix()
{
  //Print_Vars();
  for (int i = 0; i < row; ++i) {
    //loop each row/bank
    for (int  j = 0; j < column; ++j) {

      // j is only ever going to be column 0-5, needs a case .. j+6, j+12, j+18
      // bank_matrix[i][j] = RK002_paramGet(j);
      int param = 0;
      switch (i) {
        case 0:
          param = j;
          bank_matrix[0][j] = RK002_paramGet(param);
          break;
        case 1:
          param = j + 6;
          bank_matrix[1][j] = RK002_paramGet(param);
          break;
        case 2:
          param = j + 12;
          bank_matrix[2][j] = RK002_paramGet(param);
          break;
        case 3:
          param = j + 18;
          bank_matrix[3][j] = RK002_paramGet(param);
          break;

      }//end switch

      //RK002_printf("Row %d Column %d = %d", i, j, bank_matrix[i][j]);
    }
  }
}

void Indicator()
{
  RK002_sendNoteOn(15, 60, 127);
  delay(50);
  RK002_sendNoteOff(15, 60, 127);
  delay(50);
}

void Success()
{
  RK002_sendNoteOn(15, 60, 127);
  //  delay(50);
  RK002_sendNoteOn(15, 64, 127);

  RK002_sendNoteOn(15, 67, 127);
  // delay(50);
  RK002_sendNoteOff(15, 64, 127);

  RK002_sendNoteOff(15, 60, 127);

  // delay(50);
  RK002_sendNoteOff(15, 67, 127);

  //Panic();
}

void Fail()
{
  RK002_sendNoteOn(15, 67, 127);
  //delay(50);
  RK002_sendNoteOn(15, 63, 127);

  RK002_sendNoteOn(15, 60, 127);

  //delay(50);

  RK002_sendNoteOff(15, 67, 127);
  RK002_sendNoteOff(15, 63, 127);
  RK002_sendNoteOff(15, 60, 127);

  //  Panic();
}

void LearnModeTest() {
  //Print_Vars();
  unsigned long time = millis();  //time right now.

  if (LearnMode_count == 0) {
    StartTime = time;
  }
  ElapsedTime =  time - StartTime;
  LearnMode_count ++;
  //RK002_printf("StartTime = %d", StartTime);
  //RK002_printf("LearnMode_count = (LearnMode_count=%d)", LearnMode_count);
  //RK002_printf("ElapsedTime = (ElapsedTime=%d)", ElapsedTime);

  if ( LearnMode_count == 3 && ElapsedTime <= 2000) {
    ElapsedTime = 0;
    StartTime = 0;
    LearnMode_count = 0;

    if (LearnMode_active == false) {
      LearnMode_active = true;
      LearnMode_count = 0;

      //RK002_printf("LearnMode Activated");
      bank = 0;
      //Play Indicator Notes?
      Success();

    }
    else {
      LearnMode_active = false;
      LearnMode_count = 0;
      //RK002_printf("LearnMode Deactivated");
      //RK002_printf("LearnMode_active =%d", LearnMode_active);
      Refresh_BankMatrix();
      Bass_RobinPrep();
      // bank = 0;
      //what if bank 0 is empty though?
      for (byte i = 0; i < 4; i++) {
        if (Bank_Empty(i) == false ) {
          bank = i;
          break;
        }
      }
      Fail();
    }
    //return true;
  }
  else if ( ElapsedTime > 2000) {
    //RK002_printf("ElapsedTime exceeded 2000. ElapsedTime = (ElapsedTime=%d)", ElapsedTime);

    ElapsedTime = 0;
    StartTime = 0;
    LearnMode_count = 0;

    Fail();
  }
  else {
    //RK002_printf("Count is less than 3 and less than 2000");

    //return false;
  }

}


//Learn Mode
void LearnMode(byte channel, byte key, byte velocity)
{

  //bank is set,save note to bank
  unsigned Slot = 0;  //Slot is the paramater number that ends up being the first zero it finds.
  //RK002_printf("LearnMode:LearnMode entered for bank %d", bank);
  switch (bank) {
    case 0:
      //bank 1 is row 0, columns 0-5; paramater numbers 0-5;
      // unsigned Slot;
      for (int x = 0; x < 6; x++) {
        if ( RK002_paramGet(x) == 0) {
          Slot = x;
          //RK002_printf("Found slot %d empty. ", x);
          RK002_paramSet(Slot, key );
          break;
        }
        //     else(//RK002_printf("Slot %d taken in bank %d",x, bank));
      }
      //  RK002_paramSet(Slot, key );
      break;
    case 1:
      //bank 2 is row 1, columns 0-5;parameter number 6-11;
      // unsigned Slot;
      for (int x = 6; x < 12; x++) {
        if ( RK002_paramGet(x) == 0) {
          Slot = x;
          //RK002_printf("Found slot %d empty. ", x);
          RK002_paramSet(Slot, key );
          break;
        }
        //     else(//RK002_printf("Slot %d taken in bank %d",x, bank));
      }
      //  RK002_paramSet(Slot, key );
      break;
    case 2:
      //bank 3 is row 2, columns 0-5; param number 12-17;
      //  unsigned Slot;
      for (int x = 12; x < 18; x++) {
        if ( RK002_paramGet(x) == 0) {
          Slot = x;
          //RK002_printf("Found slot %d empty. ", x);
          RK002_paramSet(Slot, key );
          break;
        }
        //  else(//RK002_printf("Slot %d taken in bank %d",x, bank));
      }
      //  RK002_paramSet(Slot, key );
      break;
    case 3:
      //bank 4 is row 3, columns 0-5; param number 18-23;
      //unsigned Slot;
      for (int x = 18; x < 24; x++) {
        if ( RK002_paramGet(x) == 0) {
          Slot = x;
          //RK002_printf("Found slot %d empty. ", x);
          RK002_paramSet(Slot, key );
          break;
        }
        //  else(//RK002_printf("Slot %d taken in bank %d",x, bank)
        //  );
      }
      //  RK002_paramSet(Slot, key );
      break;
  }
  ////RK002_printf("Slot/Parameter %d and key %d Set for bank %d", Slot, key, bank);
  //  Refresh_BankMatrix();
  //    List_Params();

}

void List_Params()
{
  for (int i = 0; i < 24; i++) {
    int param = RK002_paramGet(i);
    //RK002_printf("Parameter %d = %d", i, param);
  }
}

void SetBank(byte key)
{ // Middle C is Note #60 aka C4//
  switch (key) {

    case 60:
      bank = 0;
      bankset = true;
      //RK002_printf("Bank %d Set from key %d", bank, key);
      break;
    case 61:
      bank = 1;
      bankset = true;
      //RK002_printf("Bank %d Set from key %d", bank, key);
      break;
    case 62:
      bank = 2;
      bankset = true;
      //RK002_printf("Bank %d Set from key %d", bank, key);
      break;
    case 63:
      bank = 3;
      bankset = true;
      //RK002_printf("Bank %d Set from key %d", bank, key);
      break;
  }

}
void ClearBank()
{
  // Clear bank of notes
  // needs work.
  for (int i = 0; i < 6; i++) {
    bank_matrix[bank][i] = 0;

    //which parameter do I set to zero? use the bank
    switch (bank) {
      case 0:
        RK002_paramSet(i, 0 );
        break;
      case 1:
        RK002_paramSet(i + 6, 0 );
        break;
      case 2:
        RK002_paramSet(i + 12, 0 );
        break;
      case 3:
        RK002_paramSet(i + 18, 0 );
        break;
    }
    //RK002_printf("Bank %d Cleared", bank);

    //clear the RoundRobin_Chord notes for the bass feature and the w_total for good measure
    for (int i = 0; i < 6; i++) {
      RoundRobin_Chord[i] = 0;
    }
    w_total = 0;

    Success();
  }
}
bool ClearBank_Test(unsigned long starttime, unsigned long endtime) {
  if (endtime - starttime >= 5000) {
    PressBegin = 0;
    PressEnd = 0;

    return true;
  }
  else {
    return false;
  }
}

//Get out of everything;
void Panic() {
  // LearnMode_active = false;
  //  LearnMode_count = 0;
  //  bankset = false;
  //  ElapsedTime = 0;
  //  StartTime = 0;

  //all notes off, all channels

  for (int i = 0; i < 127; ++i) {
    for (int j = 0; j < 16; ++j) {
      RK002_sendNoteOff(j, i, 0);
    }
  }


}


boolean RK002_onProgramChange(byte channel, byte nr)
{
  //change Yamaha PRG CHG to KORG PRG CHG
  return true;
}

boolean RK002_onNoteOn(byte channel, byte key, byte velocity)
{ //RK002_printf("OnNoteOn  (chn=%d, key=%d, vel=%d)", channel, key, velocity);
  OnVelocity = velocity;
  OrigKey = key;
  //would need to check to see if learn Mode is active
  if (LearnMode_active == true) {
    // what if you wanna change banks?
    PressBegin = millis();  //time right now
    //RK002_printf("PressBegin = (PressBegin=%d)", PressBegin);
    return true;
  }
  else {  //Perform mode
    if (channel == chord_channel ) {
      // if(key == 0){PressBegin = millis();} //start timer for turning off chords feature;
      //it should be noted that if chords is off, then the bank can't be advanced.
      if (chords == true) {
        Round_Robin(); Bass_RobinPrep();   Bank_Advance();  return false;
      }
      else {
        return true;
      }
    }
    else if ( channel == note_channel) {
      // if(key == 0){PressBegin = millis();} //start timer for turning off note feature;

      if (melody == true) {
        Melody_Robin(velocity);
        return false;
      }
      else {
        return true;
      }

    }
    else if ( channel == bass_channel) {
      //if(key == 0){PressBegin = millis();} //start timer for turning off chords feature;

      if (bass == true) {
        Bass_Robin();
        return false;
      }
      else {
        return true;
      }
    }
    else {
      return true;
    }
  }

}




boolean RK002_onNoteOff(byte channel, byte key, byte velocity)
{
  OrigKey_off = key;
  if (LearnMode_active == true) {
    PressEnd = millis(); //time now
    //RK002_printf("PressEnd = (PressEnd=%d)", PressEnd);
    if (ClearBank_Test(PressBegin, PressEnd) == true  ) {
      ClearBank();
    }
    else {
      LearnMode(channel, key, OnVelocity);
    }
    return true;
  }//end if learnmode active
  else {
    if (channel == chord_channel ) {
      //if(key == 0){
      //     PressEnd = millis(); //time now
      //      if (ClearBank_Test(PressBegin, PressEnd) == true  ) {
      //        if(chords == true){chords = false; Round_RobinOff();} //Fail(); }
      //       else{chords = true; //play note off for note 0;
      //      RK002_sendNoteOff(chord_channel, 0, 127);
      // Success(); }
      // }
      //    }
      //}
      if (chords == true) {
        Round_RobinOff();
        return false;
      }
      else {
        return true;
      }
    }
    else if (channel == note_channel) {
      // if(key == 0){
      //  PressEnd = millis(); //time now
      //        if (ClearBank_Test(PressBegin, PressEnd) == true  ) {
      //    if(melody == true){melody = false; Melody_RobinOff();}    //Fail(); }
      //    else{melody = true;
      //   RK002_sendNoteOff(note_channel, 0, 127);
      //Success(); }
      //   }
      //  }
      // }

      if (melody == true) {
        Melody_RobinOff();
        return false;
      }
      else {
        return true;
      }
    }
    else if (channel == bass_channel) {

      //  if(key == 0){
      //    PressEnd = millis(); //time now
      //          if (ClearBank_Test(PressBegin, PressEnd) == true  ) {
      //            if(bass == true){bass = false; Bass_RobinOff();}  //Fail(); }
      //      else{bass = true;
      //     RK002_sendNoteOff(bass_channel, 0, 127);
      // Success(); }
      //    }

      //  }
      // }
      if (bass == true) {
        Bass_RobinOff();
        return false;
      }
      else {
        return true;
      }
    }
    else {
      return true;
    }
    // Bank_Advance();  Need a way to change the bank only when chords are hit but fix the chord cut offs too.
    //return false;


    //RK002_printf("NoteOff: LearnMode_status = %d", LearnMode_active);
  }
}
//

//Check for empty bank
bool Bank_Empty(byte b)
{
  bool statuss;

  for (byte i = 0; i < 6; i++)
  {
    if ( bank_matrix[b][i] != 0) {
      //RK002_printf("Bank_Empty: bank %d column %d not zero", b, i);
      statuss =  false;
      break;
    }
    else {
      //RK002_printf("Bank_Empty: bank %d column %d zero", b, i);
      statuss =  true;
    }
  }

  return statuss;
}

void Bank_Advance_LM()
{
  if (bank == 3)
    bank = 0;

  else
    bank = bank + 1;

}


void Bank_Advance()
{
  byte counter = 0;
  for (int i = bank; i < 4; i++) {

    if (counter > 4) {
      //RK002_printf("All banks empty");
      break;
    }

    if (i == 3) {
      i = -1;
      //RK002_printf("3 was set to %d", i);
      //break;
    }

    if (Bank_Empty(i + 1) == false ) {
      bank = i + 1;
      //RK002_printf("bank set to bank %d", bank);
      break;
    }
    counter++;
  }



}

void Feature_toggle(byte channel, byte value)
{
  switch (channel) {
    case 0:
      if (value == 0) {
        chords = false; break;
      } else if (value == 127) {
        chords = true; break;
      }
    case 1:
      if (value == 0) {
        melody = false; break;
      } else if (value == 127) {
        melody = true; break;
      }
    case 2:
      if (value == 0) {
        bass = false; break;
      } else if (value == 127) {
        bass = true; break;
      }



  }



}


boolean RK002_onControlChange(byte channel, byte nr, byte value)
{

  if (LearnMode_active == true && nr == 51 && value == 127)
  { Bank_Advance_LM();
    for (byte i = 0; i < bank + 1; i++)
    {
      Indicator();
    }
    //RK002_printf("OnControlChange: bank set to bank %d", bank);
   // return false;
  }
  else if ( nr == 50 && LearnMode_active == false) {
    Feature_toggle(channel, value);
  }
  else if ( nr == 54 && value == 127)
  {
    LearnModeTest();
   // return false;
  }

  return true;
}

boolean RK002_onStop()
{
  //RK002_printf("OnStop");
  //RK002_printf("LearnModeKey = (LearnModeKey=%d)", LearnModeKey );
  //Is LearnMode set to Stop msg?
  // if ( LearnModeKey == Stop) {
  //   LearnModeTest();

  // } //end else
  return true;
}

/*
   so this is getting a bit out of hand.  the assumption of the root note is the first problem
   the 2nd is that there's no great way to define if the chord is major or minor
   also, the use of pointers seems wrong.

*/
// might rethink using RoundRobin_Chord and use the bank_matrix to prepopulate notes for all chords ahead of time.  Run this for each bank basically.
void Bass_RobinPrep()
{
  if (Bank_Empty(bank) == false) {

    //filling up the RoundRobin_chord first;
    for (byte i = 0; i < 6; i++)
    { byte note = bank_matrix[bank][i];
      if (note != 0) {
        RoundRobin_Chord[i] = note;
      }
    }


    int count = 0;
    //count the non-zero members of the array
    for (int i = 0; i < 6; i++)
    { if (RoundRobin_Chord[i] != 0)count++;
    }
    //RK002_printf("Chord has %d members", count);

    //find the lowest note within RoundRobin_Chord();
    int index = 0 ;
    for (int i = 0; i < count; i++)
    {
      if (RoundRobin_Chord[i] < RoundRobin_Chord[index] ) {
        index = i;

      }
    }
    //RK002_printf("Lowest Note = %d", RoundRobin_Chord[index]);
    // bass guitar range is 28-67;

    byte *Lowest = &RoundRobin_Chord[index];
    //use the lowest note -12 as the root ?

    //normalize
    while (*Lowest > 33) {
      *Lowest = *Lowest - 12;
    }
    *Lowest = *Lowest + 12;

    byte root = *Lowest;
    //RK002_printf("root %d,Lowest %d", root, *Lowest);
    byte fiv_u = root + 7;
    byte fiv_d = root - 7;
    byte otv_u = root + 12;
    byte for_u = root + 5;
    byte for_d = root - 5;

    //RK002_printf("5up %d, 5dn %d, ocv %d", fiv_u, fiv_d, otv_u);
    //byte  *b_note[] = {&root, &fiv_u, &fiv_d, &otv_u, &for_u, &for_d};
    byte weight[] = {10, 5, 5, 7, 2, 2};

    w_total = 0;
    byte weight_count = sizeof(weight) / sizeof(weight[0]);
    //RK002_printf("Weight size %d", weight_count);

    // get the sum of weights
    for (int i = 0; i < weight_count; i++)
    { w_total = w_total + weight[i];
    }
    //RK002_printf("Total of weights %d", w_total);

    //byte pool[w_total];
    //  byte b_note_size = sizeof(*b_note) / sizeof(*b_note[0]);
    //  //RK002_printf("Total b_notes: %d",b_note_size);

    // pick a random number from 1 to the total+1;
    //int pick = random(1,w_total+1);


    for (byte j = 0; j < w_total; j++) {
      switch (j) {
        case 0 ... 9 :
          pool[j] = root;
          //RK002_printf("Case %d, %d added", j, root);
          break;
        case 10 ... 14 :
          pool[j] = fiv_u;
          //RK002_printf("Case %d, %d added", j, fiv_u);
          break;
        case 15 ... 19 :
          pool[j] = fiv_d;
          //RK002_printf("Case %d, %d added", j, fiv_d);
          break;
        case 20 ... 26 :
          pool[j] = otv_u;
          //RK002_printf("Case %d, %d added", j, otv_u);
          break;
        case 27 ... 28 :
          pool[j] = for_u;
          //RK002_printf("Case %d, %d added", j, for_u);
          break;
        case 29 ... 30 :
          pool[j] = for_d;
          //RK002_printf("Case %d, %d added", j, for_d);
          break;
      }

    }


  }
}

void Bass_Robin()
{
  int pick = random(0, w_total);

  bass_note = pool[pick];


  // max size of the OriGen_KeyMap sub array is 6 notes.  One Note should not trigger more than 6 members, 2-3 is probably physical limit.
  for (byte i = 0; i < 6; i++) {

    if (OriGen_KeyMap[OrigKey][i] == 0) {
      OriGen_KeyMap[OrigKey][i] = bass_note;
      break;
    }

  }



  RK002_sendNoteOn(bass_channel, bass_note, OnVelocity);
}

void Bass_RobinOff()
{

  // Find a way to get the first note in the sub array for the orig key and sent the note off for it.
  /* for ( byte i = 0; i < 6; i++)
    {
     if (OriGen_KeyMap[OrigKey_off][i] != 0) {
       byte note = OriGen_KeyMap[OrigKey_off][i];
       RK002_sendNoteOff(bass_channel, note, OnVelocity);
       OriGen_KeyMap[OrigKey_off][i] = 0;
     }

    }*/

  byte note = OriGen_KeyMap[OrigKey_off][0];
  RK002_sendNoteOff(bass_channel, note, OnVelocity);
  OriGen_KeyMap[OrigKey_off][0] = 0;
  //how to pass one row of a 2d array to a function expecting an array only?
  pushZerosToEnd(OriGen_KeyMap[OrigKey_off], 6);

  //RK002_sendNoteOff(bass_channel, bass_note, OnVelocity);

}

void banksort(int bank, int count) {
  for (int i = 0; i < (count - 1); i++) {
    for (int o = 0; o < (count - (i + 1)); o++) {
      if (bank_matrix[bank][o] > bank_matrix[bank][o + 1]) {
        int t = bank_matrix[bank][o];
        bank_matrix[bank][o] = bank_matrix[bank][o + 1];
        bank_matrix[bank][o + 1] = t;
      }
    }
  }
}

void Down()
{
  int count = 0;
  for (int i = 0; i < 6; i++) {
    byte note = bank_matrix[bank][i];
    if (note != 0) {
      count++;
    }
  }

  banksort(bank, count);
  //RK002_printf("Down note is %d", up_note);
  if (count != 0) {
    if (up_note == 0) {
      up_note = count - 1;
    }
    else {
      up_note = up_note - 1;
    }
  }
}
void Up()
{
  int count = 0;
  for (int i = 0; i < 6; i++) {
    byte note = bank_matrix[bank][i];
    if (note != 0) {
      count++;
    }
  }

  banksort(bank, count);
  for (int i = 0; i < 6; i++) {
    //RK002_printf("Bank %d Row %d", bank,  bank_matrix[bank][i] );

  }
  //RK002_printf("Upnote is %d", up_note);

  if (count != 0) {
    up_note = up_note + 1;
    if ( up_note == count) {
      up_note = 0;
    }
  }
}


void Melody_Robin(byte velocity)
{
  //if the previous noteoff didn't happen and the note hangs, but that's polyphony?
  //RK002_printf("Bank is %d", bank);
  //Melody_RobinOff();

  if (velocity > 90) {
    Up();
  }
  else {
    Down();
  }

  // used to get value of count for the random feature below;
  /* int count = 0;
    for (int i = 0; i < 6; i++) {
     byte note = bank_matrix[bank][i];
     if (note != 0) {
       count++;
     }
    }
    //RK002_printf("Bank size is %d", count);
  */
  //MelodyRobin_Note = bank_matrix[bank][random(0, count)];
  MelodyRobin_Note = bank_matrix[bank][up_note];


  // OriGen_KeyMap[OrigKey] = MelodyRobin_Note;

  // max size of the OriGen_KeyMap sub array is 6 notes.  One Note should not trigger more than 6 members, 2-3 is probably physical limit.
  for (byte i = 0; i < 6; i++) {

    if (OriGen_KeyMap[OrigKey][i] == 0) {
      OriGen_KeyMap[OrigKey][i] = MelodyRobin_Note;
      break;
    }

  }

  if (MelodyRobin_Note != 0) {
    RK002_sendNoteOn(note_channel, MelodyRobin_Note, OnVelocity);
  }

  //RK002_printf("Note %d ", MelodyRobin_Note);
}

void Melody_RobinOff()
{
  // byte note = MelodyRobin_Note;
  //byte note = OriGen_KeyMap[OrigKey_off];

  // Find a way to get the first note in the sub array for the orig key and sent the note off for it.
  /*for ( byte i = 0; i < 6; i++)
    {
    if (OriGen_KeyMap[OrigKey_off][i] != 0) {
      byte note = OriGen_KeyMap[OrigKey_off][i];
      RK002_sendNoteOff(note_channel, note, OnVelocity);
      OriGen_KeyMap[OrigKey_off][i] = 0;
    }
    }*/

  byte note = OriGen_KeyMap[OrigKey_off][0];
  RK002_sendNoteOff(note_channel, note, OnVelocity);
  OriGen_KeyMap[OrigKey_off][0] = 0;
  //how to pass one row of a 2d array to a function expecting an array only?
  pushZerosToEnd(OriGen_KeyMap[OrigKey_off], 6);


  //  RK002_sendNoteOff(note_channel, note, OnVelocity);
}


// Function which pushes all zeros to end of an array. n is the count of array elements
void pushZerosToEnd( byte arr[], int n)
{
  int count = 0;  // Count of non-zero elements

  // Traverse the array. If element encountered is non-
  // zero, then replace the element at index 'count'
  // with this element
  for (int i = 0; i < n; i++)
    if (arr[i] != 0)
      arr[count++] = arr[i]; // here count is
  // incremented

  // Now all non-zero elements have been shifted to
  // front and  'count' is set as index of first 0.
  // Make all elements 0 from count to end.
  while (count < n)
    arr[count++] = 0;
}

//round robin fuction
void Round_Robin()
{
  //filling up the RoundRobin_chord first;
  for (byte i = 0; i < 6; i++)
  { byte note = bank_matrix[bank][i];
    if (note != 0) {
      RoundRobin_Chord[i] = note;
    }
  }

  for (byte i = 0; i < 6; i++)
  { byte note = bank_matrix[bank][i];
    if (note != 0) {
      RK002_sendNoteOn(chord_channel, note, OnVelocity);
    }
  }
  //this just needs to hold the the bank once. break after finding a spot;
  for (byte j = 0; j < 6; j++)
  {
    if (OriGen_ChordMap[OrigKey][j] == 0)
    {
      OriGen_ChordMap[OrigKey][j] = bank + 1;
      // OriGen_ChordMap[OrigKey][j] = bank;  //8.16.20 edit.
      break;
    }

  }
}

void Round_RobinOff()
{ //just figure out the bank that was played for the orig note sent. use FIFO.
  /*
    for (byte i = 0; i < 6; i++)
    {
    if (OriGen_ChordMap[OrigKey_off][i] != 0)
    {
      byte note_bank = OriGen_ChordMap[OrigKey_off][i] - 1;
      // thisis the bank-1 we want to note offs
      for (byte j = 0; j < 6; j++)
      {
        byte note = bank_matrix[note_bank][j];
        if (note != 0)
        { RK002_sendNoteOff(chord_channel, note, OnVelocity);
        }
      }
      //set the sub array column 0 to zero
      OriGen_ChordMap[OrigKey_off][i] = 0;

    }

    }*/  //8.16.20 edit
  byte note_bank = OriGen_ChordMap[OrigKey_off][0] - 1 ;
  //RK002_printf("note_bank is %d",note_bank);
  OriGen_ChordMap[OrigKey_off][0] = 0;
  for (byte j = 0; j < 6; j++)
  {
    byte note = bank_matrix[note_bank][j];
    if (note != 0)
    { RK002_sendNoteOff(chord_channel, note, OnVelocity);
    }
  }
  // now move the bank values up
  //push zeros to end is really a way deal with >1 noteOnds / bank being triggered.  chordmap is really a 'bank hopper', not hopper as in jump,
  // hopper as in an ordered queue system.  FIFO.
  // meaning that its a 1-based index to keep track of what bank should be triggered off.
  pushZerosToEnd(OriGen_ChordMap[OrigKey_off], 6);



}

boolean RK002_onClock()
{
  return true;
}


boolean RK002_onStart()
{
  return true;
}

boolean RK002_onContinue()
{

  return true;
}

boolean RK002_onActiveSensing()
{
  return true;
}

boolean RK002_onReset()
{
  return true;
}

void setup() {
  // put your setup code here, to run once:

  //todo:  code a way to change LearnMode access/exit
  Refresh_BankMatrix();
  /*
    for (int i = 0; i < 24; i++) {
      int param = RK002_paramGet(i);
      //RK002_printf("Parameter %d = %d", i, param);

    }
  */
}//end setup

void loop() {
  // put your main code here, to run repeatedly:

}//end loop
