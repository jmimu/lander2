#! /usr/bin/python

#Copyright 2013-2014 jmimu (jmimu@free.fr)
#Converts string representing music into asm declaration for sega master system (to use with jmimu's code)
#music representation: note-octave-alteration-duration-volume
#examples "g3_0017F", "f5#00058"
#  note: a to f
#  octave: 1 to 7, octave changes after b!
#  alteration: "_" or "#"
#  previous durations : q,t,h,1,p,2,3,4 (quarter,third, half, 1 time, pointed (1.5), 2 times, 3 times, 4 times)
#  durations are written in 4  decimal digits representing 12th of beat
#  volume: one hex (uppercase) value, 0 = nosound, F = max
#  vibrato: ~ at the end (freq and amplitude fixed, make 3 notes with variation of frequence in asm)
#every note is 5-char long. => todo: enable fast notation!
#For pause : "Ppp"+duration+volume (volume is unused)
#For end : "End"+duration+volume (duration and volume is unused)

#TODO : make echo, add output volume parameter

fps=60

#interpret_melody() automatically creates volume enveloppe  for each note, using this parameter:

pseudo_frame_size=1 #to minimize memory used


tempo=120
time_unit=60*fps/tempo #number of frames for one time

#TODO: chek each channel has the same size! (add bank at the end)
#!!!!!!!!!!!!!!!! make sure every possible note duration is a multiple of frames withe tempo=120, 100, 80 or 60
#


#ADSR envelop (duration is in frames = time*time_unit)
attack_duration=time_unit/10
decay_duration=time_unit/20
sustain_ratio=0.8
release_duration=time_unit/2
#(release is only if pause after the note)

print("ADSR: ",attack_duration," ",decay_duration," ",sustain_ratio," ",release_duration)

SMS_NTSC={
"Ppp":"$00,$00","End":"$ff,$ff","a1_":"$03,$f9","a1#":"$03,$c0","b1_":"$03,$8a","c2_":"$03,$57","c2#":"$03,$27","d2_":"$02,$fa","d2#":"$02,$cf","e2_":"$02,$a7","f2_":"$02,$81","f2#":"$02,$5d","g2_":"$02,$3b","g2#":"$02,$1b","a2_":"$01,$fc","a2#":"$01,$e0","b2_":"$01,$c5","c3_":"$01,$ac","c3#":"$01,$94","d3_":"$01,$7d","d3#":"$01,$68","e3_":"$01,$53","f3_":"$01,$40","f3#":"$01,$2e","g3_":"$01,$1d","g3#":"$01,$0d","a3_":"$00,$fe","a3#":"$00,$f0","b3_":"$00,$e2","c4_":"$00,$d6","c4#":"$00,$ca","d4_":"$00,$be","d4#":"$00,$b4","e4_":"$00,$aa","f4_":"$00,$a0","f4#":"$00,$97","g4_":"$00,$8f","g4#":"$00,$87","a4_":"$00,$7f","a4#":"$00,$78","b4_":"$00,$71","c5_":"$00,$6b","c5#":"$00,$65","d5_":"$00,$5f","d5#":"$00,$5a","e5_":"$00,$55","f5_":"$00,$50","f5#":"$00,$4c","g5_":"$00,$47","g5#":"$00,$43","a5_":"$00,$40","a5#":"$00,$3c","b5_":"$00,$39","c6_":"$00,$35","c6#":"$00,$32","d6_":"$00,$30","d6#":"$00,$2d","e6_":"$00,$2a","f6_":"$00,$28","f6#":"$00,$26","g6_":"$00,$24","g6#":"$00,$22","a6_":"$00,$20","a6#":"$00,$1e","b6_":"$00,$1c","c7_":"$00,$1b","c7#":"$00,$19","d7_":"$00,$18","d7#":"$00,$16","e7_":"$00,$15","f7_":"$00,$14","f7#":"$00,$13","g7_":"$00,$12","g7#":"$00,$11"}


SMS_PAL ={
"Ppp":"$00,$00","End":"$ff,$ff","a1_":"$03,$f0","a1#":"$03,$b7","b1_":"$03,$82","c2_":"$03,$4f","c2#":"$03,$20","d2_":"$02,$f3","d2#":"$02,$c9","e2_":"$02,$a1","f2_":"$02,$7b","f2#":"$02,$57","g2_":"$02,$36","g2#":"$02,$16","a2_":"$01,$f8","a2#":"$01,$dc","b2_":"$01,$c1","c3_":"$01,$a8","c3#":"$01,$90","d3_":"$01,$79","d3#":"$01,$64","e3_":"$01,$50","f3_":"$01,$3d","f3#":"$01,$2c","g3_":"$01,$1b","g3#":"$01,$0b","a3_":"$00,$fc","a3#":"$00,$ee","b3_":"$00,$e0","c4_":"$00,$d4","c4#":"$00,$c8","d4_":"$00,$bd","d4#":"$00,$b2","e4_":"$00,$a8","f4_":"$00,$9f","f4#":"$00,$96","g4_":"$00,$8d","g4#":"$00,$85","a4_":"$00,$7e","a4#":"$00,$77","b4_":"$00,$70","c5_":"$00,$6a","c5#":"$00,$64","d5_":"$00,$5e","d5#":"$00,$59","e5_":"$00,$54","f5_":"$00,$4f","f5#":"$00,$4b","g5_":"$00,$47","g5#":"$00,$43","a5_":"$00,$3f","a5#":"$00,$3b","b5_":"$00,$38","c6_":"$00,$35","c6#":"$00,$32","d6_":"$00,$2f","d6#":"$00,$2d","e6_":"$00,$2a","f6_":"$00,$28","f6#":"$00,$25","g6_":"$00,$23","g6#":"$00,$21","a6_":"$00,$1f","a6#":"$00,$1e","b6_":"$00,$1c","c7_":"$00,$1a","c7#":"$00,$19","d7_":"$00,$18","d7#":"$00,$16","e7_":"$00,$15","f7_":"$00,$14","f7#":"$00,$13","g7_":"$00,$12","g7#":"$00,$11"}


class Note(object):
  def __init__(self,_text):
    self.text=_text
    item=self.text.strip()
    self.tone=item[0:3]
    self.duration=int(item[3:7],10)
    self.volume=int(item[7],16)


"""
  The melody is interpreted as an analog function, stored as (time,volume,note) 
  The function is linear between the store points
  and its mean value is computed for every pseudo-frame
"""
class Melody(object):
  def __init__(self,_name,_SMS_norm):
    self.name=_name
    self.SMS_norm=_SMS_norm
    self.melodytext=""
    self.length=0
    self.all_notes=[]
    self.analog_function=[]
  def interpret(self,_melodytext):
    self.melodytext=_melodytext
    self.length=0
    self.all_notes=[]
    self.analog_function=[]
    current_time=0
    for item in self.melodytext.split():
      self.all_notes.append(Note(item))
    for i in range(len(self.all_notes)):
      note=self.all_notes[i]
      start_current_time=current_time
      print "item : ",note.text,note.tone,self.SMS_norm[note.tone],note.duration
      #check if note is long enougth
      if (note.duration<attack_duration+decay_duration):
        print("Error! ",note.text," is too short for ADSR!!!")
      
      #add first point of the note (volume 0)
      self.analog_function.append( (current_time,0,note.tone) )
      #add end of attack
      current_time+=attack_duration
      self.analog_function.append( (current_time,note.volume,note.tone) )
      #add end of decay
      current_time+=decay_duration
      self.analog_function.append( (current_time,note.volume*sustain_ratio,note.tone) )
      #add end of note
      current_time+=note.duration-attack_duration-decay_duration
      self.analog_function.append( (current_time,note.volume*sustain_ratio,note.tone) )
      #if next is a pause, add release time
      if ((i<len(self.all_notes)-1) and (self.all_notes[i+1].tone=="Ppp")):
          current_time+=release_duration
          self.analog_function.append( (current_time,0,"Ppp") )
          #add the remaining pause time
          current_time+=self.all_notes[i+1].duration-release_duration
    self.length=current_time
    print(self.analog_function)
      
    
  def setLength(self,aim_length):
    #TODO: if longer than length, add release time of last note, then pause
    
  def toASM(self):
    output=self.name+"_start:\n  .db "
    output+=SMS_norm["End"]+"\n  ;total {} frames\n{}_end:\n".format(self.length,self.name)
    return output

    



if __name__ == '__main__':
  
  test1="f3_0017A f3_0006A a3_0012A f3_0012A e3_0023A a3_0006A Ppp0017F d3_0017A d3_0006A f3_0012A d3_0012A c3_0023A Ppp0023F a2#0017A a2#0006A c3_0012A a2#0012A a2_0023A f3_0023A e3_0017A e3_0006A f3_0012A g3_0012A f3_0023A Ppp0021F f5_0011A "
  test2="f4_00067 a4_00067 c5_00067 a4_00067 f4_00067 a4_00067 c5_00067 a4_00067 e4_00067 a4_00067 c5_00067 a4_00067 e4_00067 a4_00067 c5_00067 a4_00067 d4_00067 f4_00067 a4_00067 f4_00067 d4_00067 f4_00067 a4_00067 f4_00067 c4_00067 f4_00067 a4_00067 f4_00067 c4_00067 f4_00067 a4_00067 f4_00067 a3#00067 d4_00067 f4_00067 d4_00067 a3#00067 d4_00067 f4_00067 d4_00067 a3_00067 c4_00067 f4_00067 c4_00067 a3_00067 c4_00067 f4_00067 c4_00067 g3_00067 c4_00067 f4_00067 c4_00067 g3_00067 c4_00067 f4_00067 c4_00067 f3_00067 a3_00067 c4_00067 f4_00067 a4_00067 c5_00067 f5_00117 "
  
  melody1=Melody("end_music_ch1",SMS_NTSC)
  melody1.interpret(test1)
  

  
  

