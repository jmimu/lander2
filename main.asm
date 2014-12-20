;bloup
;
;Copyright (C) 2013  jmimu (jmimu@free.fr)
;
;This program is free software: you can redistribute it and/or modify
;it under the terms of the GNU General Public License as published by
;the Free Software Foundation, either version 3 of the License, or
;(at your option) any later version.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program.  If not, see <http://www.gnu.org/licenses/>.
;==============================================================


;==============================================================
; WLA-DX banking setup
; Note that this is a frame 2-only setup, allowing large data
; chunks in the first 32KB.
;==============================================================
.memorymap
   defaultslot 0
   ; ROM area
   slotsize        $8000
   slot            0       $0000
   slotsize        $4000
   slot            1       $8000
   ; RAM area
   slotsize        $2000
   slot            2       $C000
   slot            3       $E000
.endme

.rombankmap
   bankstotal 1
   banksize $8000
   banks 1
.endro




;==============================================================
; constants
;==============================================================
;demo
.define forest_1st_tile $2
.define forest_scroll_1st_tile_from_forest_start $40
.define forest_anim_steps $8
.define bike_tile_number $20
.define bike_pedal_tile_number $32
.define bike_pedal_anim_steps $4

;game
.define number_of_empty_tiles 13;tile 13 and more make collisions
.define last_full_tile 34;tile 35 and more make no collisions
.define digits_tile_number $A7 ;position of "0" in vram
.define fire_tile_number $81
.define explosion_tile_number $86
.define fuel_tile_number $84
.define rocket_tile_number $8C
.define landing_tile_number $27;TODO: remove, use collision data
.define guy_tile_number $92
.define diff_tile_ascii 119 ;difference between index in tiles and in ascii ("A" tile number -65)
.define fuel_use $-70 ;$-80
.define speedX_tolerance $40 ;must be < $80 !
.define speedY_tolerance $40
.define level_mem_size 1824 ;size of 1 palette + 1 tilemap
.define number_of_levels 5 ;


;==============================================================
; RAM section
;==============================================================
.ramsection "variables" slot 2
  new_frame                     db ; 0: no; 1: yes
  PauseFlag db ;1 if pause
  ;demo variables
  forest_anim_step dw;from 0 to forest_anim_steps*$100
  pedal_anim_step dw;from 0 to forest_anim_steps*$100
  Xscroll dw
  posX                     dw ; multiplied by 2^8
  posY                     dw ; multiplied by 2^8
  number_of_sprites     db ; number of sprites to draw this frame
  ;game
  speedX                     dw ; multiplied by 2^8
  speedY                     dw ; multiplied by 2^8
  ;posX                     dw ; multiplied by 2^8
  ;posY                     dw ; multiplied by 2^8
  ;number_of_sprites     db ; number of sprites to draw this frame
  rocket_fuel         dw 
  current_level db
  already_lost db ;0 if not, 1 if lost at least 1 time
  goto_level db ;0 if no need to change level, n to enter level n
  star_color dw ;color used: bright and yellow
  ;PauseFlag db ;1 if pause

  ;music
  music1_start_ptr         dw ;pointer
  music1_current_ptr         dw ;pointer
  music1_tone_duration         db ;when 0 got to next tone
  music1_current_tone         dw ;value (for debug)
  music2_start_ptr         dw ;pointer
  music2_current_ptr         dw ;pointer
  music2_tone_duration         db ;when 0 got to next tone
  music2_current_tone         dw ;value (for debug)
  music3_start_ptr         dw ;pointer
  music3_current_ptr         dw ;pointer
  music3_tone_duration         db ;when 0 got to next tone
  music3_current_tone         dw ;value (for debug)
  drum_start_ptr         dw ;pointer
  drum_current_ptr         dw ;pointer
  drum_tone_duration         db ;when 0 got to next tone
  drum_current_tone         db ;value (for debug)

.ends


;==============================================================
; SDSC tag and SMS rom header
;==============================================================
.sdsctag 1.2,"Lander2 v0.1","bloupbloup","jmimu"

.bank 0 slot 0
.org $0000
;==============================================================
; Boot section
;==============================================================
    di              ; disable interrupts
    im 1            ; Interrupt mode 1
    jp main         ; jump to main program


.org $0038
;==============================================================
; Vertical Blank interrupt
;==============================================================
    push af
      in a,($bf);clears the interrupt request line from the VDP chip and provides VDP information
      ;do something only if vblank (we have only vblank interrupt, so nothing to do)     
      ld a,1
      ld (new_frame),a
    pop af
    ei ;re-enable interrupt
    reti


.org $0066
;==============================================================
; Pause button handler
;==============================================================
    call CutAllSound
    
    ld a,(PauseFlag) ;taken from Heliophobe's SMS Tetris 
    xor $1  ;Just a quick toggle
    ld (PauseFlag),a
  retn


;inclusions
.section "misc" free ;TODO : a section for every file!
.include "fnc_init.inc"
.include "fnc_sound.inc"
.include "fnc_sprites.inc"
.include "fnc_demo.inc"
.include "fnc_text.inc"
.ends
.include "fnc_game.inc"

.section "main" free
;==============================================================
; Main program
;==============================================================
main:
    ld sp, $dff0 ;where stack ends ;$dff0
    
    ld a,0
    ld (PauseFlag),a

    ;==============================================================
    ; Set up VDP registers
    ;==============================================================
    call initVDP

    ;call InitializeJmimu
    
    ;run demo
    call InitializeDemo
    call RunDemo

    ; Turn screen off
    ld a,%10100000
;          |||| |`- Zoomed sprites -> 16x16 pixels
;          |||| `-- Doubled sprites -> 2 tiles per sprite, 8x16
;          |||`---- 30 row/240 line mode
;          ||`----- 28 row/224 line mode
;          |`------ VBlank interrupts
;          `------- Disable display
    out ($bf),a
    ld a,$81
    out ($bf),a
    
    ;run game
    call RunGame
end:
  jr end


;button to test in b
;ex for pad 1:
; Button 1 = %00100000 
; Button 2 = %00010000 
; up    = %00000001 
; down  = %00000010 
; left  = %00000100 
; right = %00001000 
;output: zero flag (jp nz,???)
IsButtonPressed:
    in a,($dc)
    and b
    cp  %00100000
    ret

.ends

;==============================================================
; Data
;==============================================================
.section "assets" free
.include "data_jmimu.inc"
.include "data_demo.inc"
.include "data_game.inc"
.ends



