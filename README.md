# Robinator
a sketch for the Retrokits Rk002 cable which allows you to trigger chords, melody &amp; bass parts from one key.



This project was started because I wanted to be able to trigger chords, a melody and a bass part from drum triggers.  The code allows for 2 modes: Learn Mode and Perform mode. 
Learn Mode is how the RK002 cable stores/deletes chords.  To access learn mode, the sketch looks for CC 54 ( I use and Arturia Keystep where the restart button in USB mode sends 
CC 54).  When CC 54 is pressed 3 times within 2 seconds, the cable enters learn mode.  If you have an instrument active on channel 16, you should hear a very short success "blip".  
If you fail to send CC 54 within 2 seconds you will hear a fail "blip".  Once in learn mode, you can enter up to 6 notes per chord. After you are finished with each chord, 
you must send CC 51 (the stop key on the Keystep) to advance to the next available bank.  If your MIDI device has an active instrument on channel 16, you will hear indicators when
changing banks.  The number of beeps you hear will indicate the bank you're curretnly in.  You can also delete an entire bank of notes while in learn mode.  By holding down 
any key on a keyboard for longer than 5 seconds, the bank will be deleted.  Finally to exit learn mode, simply send CC 54, three times within 2 seconds.  You will hear the fail "blip"
upon exiting learn mode.  

On channel 1 of your midi device, you should hear the first chord when you strike any key.  Subsequent key strokes will cycle through the chords.  

On channel 2 of your midi device, any key press will cylce through the notes of the current chord.  If the velocity is greater than 90, the notes will go up the chord while 
a velodity of less than 90 will go down the chord.

On channel 3 of your midi device, you will hear "weighted-random" bass notes.  this is the most experimental part of the sketch as it tries to play bass notes within the chord but 
allowing for some notes to be played more than others (root, fifth, etc).  Currently, the possible notes are based on the lowest note of the current chord and either a fifth up,
fifth down, fourth up, fourth down, root or lowest note and octave up.  

To turn off the feature on a channel, simply send CC 50 on that channel.  Note: turning off the channel 1 chord feature will stop the chords from cycling.   


Lastly, remember that the chords are empty upon first boot up and the features are all on by default.  this means you'll have to enter chords or just turn the feature off on each 
channel.  However, once entered, the chords remain 'in the cable' despite no power to the cable.  
