
.ramsection "Game variables" slot 1
  XScrollTotal             dw    ; total X scroll in level (in pixels)
  XScrollTotalMax          dw    ; maximum of XScrollTotal value (in pixels)
  XScrollVDP               db    ; VDP XScroll (in pixels)
  YScrollTotal             dw    ; total Y scroll in level (in pixels)
  YScrollTotalMax          dw    ; maximum of YScrollTotal value (in pixels)
  YScrollVDP               db    ; VDP YScroll (in pixels)
  LevelMapStart            dw    ; pointer to tilemap start (in bytes)
  NextMapDataStart         dw    ; pointer to tilemap new part to load due to scrolling (in bytes)
  LevelMapWidth            dw    ; tilemap width start (in tiles)
  YLava                    dw    ; total Y lava in level (in pixels)
  SpecialTileFunction      dw    ; pointer to function called when touching a special tile
  WaitForVBlankFunction    dw    ; pointer to function WaitForVBlank
  
  gameStatus               db    ; 0: start, 1: play, 2: lost, 3: won
  gameStatusCnt            dw    ; counter of game status
.ends


.section "game functions" free

.include "fnc_ingame.inc"

;used to fake a call to a variable function
;function address is in (hl)
CallFunctionHL:
  ;PC was already pushed by "call CallFunctionHL" !
  jp (hl)



;update v scroll (only for 32-tile wide tilemaps)
;input : c=add to v scroll (values can be -1,0,+1)
VScrollManager:
  push af
  push bc
  push hl
    ld a,c
    cp 0
    jp z,VScrollManager_end
    ld a,c
    cp 1
    jp z,VScrollManager_down
    ld a,c
    cp -1
    jp z,VScrollManager_up
    
    
  VScrollManager_down:
    ld hl,(YScrollTotal)

    ;check that YScrollTotal is not at YScrollTotalMax
    ld bc,(YScrollTotalMax)
    ld a,h
    cp b
    jr nz,+
    ld a,l
    cp c
    jr nz,+
    ;we are at max, stop scrolling
    jp z,VScrollManager_end
  +:
    ;update YScrollTotal
    inc hl
    ld (YScrollTotal),hl
    ;update posY
    ld hl,(posY)
    dec h
    ld (posY),hl
    
    ;update YScrollVDP
    ld a,(YScrollVDP)
    inc a
    ;check if YScrollVDP >223
    cp 224
    jr c,+
    ;if YScrollVDP >223
    xor a

  +:
    ld (YScrollVDP),a
    ;VDP vertical scrolling
    out ($bf),a
    ld a,%10001001
    out ($bf),a
    
    ;test if new tilemap line to load
    ld a,(YScrollVDP)
    and %00000111
    jp nz,VScrollManager_end
    ;new line to load
    
    ;compute where the new data is in rom
    ;(LevelMapStart)+((YScrollTotal)/8+24)*64
    ld hl,(YScrollTotal)
    srl h
    rr l
    srl h
    rr l
    srl h
    rr l ;hl=(YScrollTotal)/8
    ld bc,24
    add hl,bc
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl;hl=((YScrollTotal)/8+24)*64
    ld bc, (LevelMapStart)
    add hl,bc    
    ld (NextMapDataStart),hl;(NextMapDataStart)=(LevelMapStart)+((YScrollTotal)/8+24)*32
    
    ;compute where to add it in vram : ((YScrollVDP+192)/8)*64
    ld a,(YScrollVDP)
    add a,192
    ;test if carry, then a<=a-224, because of v screen loop
    jr c,+
    ;test if a>224, then a<=a-224, because of v screen loop
    cp 224
    jr nc,+
    jr VScrollManager_down_no_carry
  +:
    sub 224
  VScrollManager_down_no_carry:
    ld h,0
    ld l,a
    add hl,hl
    add hl,hl
    add hl,hl
    
    ; Set VRAM write address to name table index 0
    ; by outputting $4000 ORed with $3800+hl
    ld a,l
    out ($bf),a
    ld a,$38|$40
    add a,h
    out ($bf),a
    ; 2. Output tilemap data
    ld hl,(NextMapDataStart)
    ld bc,64  ; Counter for number of bytes to write
    -:
        ld a,(hl)    ; Get data byte
        out ($be),a
        inc hl       ; Point to next tile
        dec bc
        ld a,b
        or c
        jr nz,-
    ;end of VScrollManager_down
    jp VScrollManager_end

    

  VScrollManager_up:
    ld hl,(YScrollTotal)

    ;check that YScrollTotal is not at 0
    ld a,h
    cp 0
    jr nz,+
    ld a,l
    cp 0
    jr nz,+
    ;we are at 0, stop scrolling
    jp z,VScrollManager_end ;remove this line to enable out of memory 
  +:
    ;update YScrollTotal
    dec hl
    ld (YScrollTotal),hl
    ;update posY
    ld hl,(posY)
    inc h
    ld (posY),hl
    
    ;update YScrollVDP
    ld a,(YScrollVDP)
    cp 0
    jr nz,+
    ;check if YScrollVDP <0
    ld a,224
  +:
    dec a
    ld (YScrollVDP),a
    ;VDP vertical scrolling
    out ($bf),a
    ld a,%10001001
    out ($bf),a
    
    ;test if new tilemap line to load
    ld a,(YScrollVDP)
    and %00000111
    jp nz,VScrollManager_end
    ;new line to load

    ;compute where the new data is in rom
    ;(LevelMapStart)+((YScrollTotal-8)/8)*64
    ld hl,(YScrollTotal)
    ld bc,-$8
    add hl,bc
    srl h
    rr l
    srl h
    rr l
    srl h
    rr l ;hl=(YScrollTotal)/8
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl
    add hl, hl;hl=((YScrollTotal-8)/8+24)*64
    ld bc, (LevelMapStart)
    add hl,bc    
    ld (NextMapDataStart),hl;(NextMapDataStart)=(LevelMapStart)+((YScrollTotal-8)/8)*32
    
    ;compute where to add it in vram : ((YScrollVDP-8)/8)*64
    ld a,(YScrollVDP)
    ld b,-$8
    add a,b
    ;test if a>223
    cp 224
    jr nc,+
    jr VScrollManager_up_no_carry
  +:
    sub 32 ;make a 32 line jump to compensate vram_h=224 instead of 256
  VScrollManager_up_no_carry:
    ld h,0
    ld l,a
    add hl,hl
    add hl,hl
    add hl,hl
    
    ; Set VRAM write address to name table index 0
    ; by outputting $4000 ORed with $3800+hl
    ld a,l
    out ($bf),a
    ld a,$38|$40
    add a,h
    out ($bf),a
    ; 2. Output tilemap data
    ld hl,(NextMapDataStart)
    ld bc,64  ; Counter for number of bytes to write
    -:
        ld a,(hl)    ; Get data byte
        out ($be),a
        inc hl       ; Point to next tile
        dec bc
        ld a,b
        or c
        jr nz,-
    ;end of VScrollManager_down
    jp VScrollManager_end
  VScrollManager_end:
  pop hl
  pop bc
  pop af
ret

;difficulty in b (1 to 5)
SetDifficulty:
  push af
  push hl
    ;use difficulty 0 for debugging
    ld a,b
    cp 0
    jr nz,+
    ;difficulty 0
    ld hl,-$10;0
    ld (fuel_use),hl;$-70
    ld hl,$60
    ld (speed_pos_tolerance),hl;$40
    ld hl,-$60
    ld (speed_neg_tolerance),hl;$40
    ld hl,$-10
    ld (Xdumping),hl;$0
    ld a,1
    ld (refuel),a;$0
    ld a,0
    ld (big_rocket),a;$1
    jp SetDifficulty_end
  +:
    ld a,b
    cp 1
    jr nz,+
    ;difficulty 1
    ld hl,$-10
    ld (fuel_use),hl;$-70
    ld hl,$60
    ld (speed_pos_tolerance),hl;$40
    ld hl,-$60
    ld (speed_neg_tolerance),hl;$40
    ld hl,$-10
    ld (Xdumping),hl;$0
    ld a,1
    ld (refuel),a;$0
    ld a,0
    ld (big_rocket),a;$1
    jp SetDifficulty_end
  +:
    ld a,b
    cp 2
    jr nz,+
    ;difficulty 2
    ld hl,$-20
    ld (fuel_use),hl;$-70
    ld hl,$60
    ld (speed_pos_tolerance),hl;$40
    ld hl,-$60
    ld (speed_neg_tolerance),hl;$40
    ld hl,$-5
    ld (Xdumping),hl;$0
    ld a,1
    ld (refuel),a;$0
    ld a,0
    ld (big_rocket),a;$1
    jr SetDifficulty_end
  +:
    ld a,b
    cp 3
    jr nz,+
    ;difficulty 3
    ld hl,$-40
    ld (fuel_use),hl;$-70
    ld hl,$50
    ld (speed_pos_tolerance),hl;$40
    ld hl,-$50
    ld (speed_neg_tolerance),hl;$40
    ld hl,$0
    ld (Xdumping),hl;$0
    ld a,1
    ld (refuel),a;$0
    ld a,0
    ld (big_rocket),a;$1
    jr SetDifficulty_end
  +:
    ld a,b
    cp 4
    jr nz,+
    ;difficulty 4
    ld hl,$-50
    ld (fuel_use),hl;$-70
    ld hl,$50
    ld (speed_pos_tolerance),hl;$40
    ld hl,-$50
    ld (speed_neg_tolerance),hl;$40
    ld hl,$0
    ld (Xdumping),hl;$0
    ld a,0
    ld (refuel),a;$0
    ld a,1
    ld (big_rocket),a;$1
    jr SetDifficulty_end
  +:
    ld a,b
    cp 5
    jr nz,SetDifficulty_end
    ;difficulty 5
    ld hl,$-70
    ld (fuel_use),hl;$-70
    ld hl,$40
    ld (speed_pos_tolerance),hl;$40
    ld hl,-$40
    ld (speed_neg_tolerance),hl;$40
    ld hl,$0
    ld (Xdumping),hl;$0
    ld a,0
    ld (refuel),a;$0  
    ld a,1
    ld (big_rocket),a;$1
  SetDifficulty_end:
  pop hl
  pop af
  ret


RunGame:
    ; setup screen (VDP register 0)
    ld a,%01000110
;         |||||||`- External Sync.
;         ||||||`-- 1
;         |||||`--- 1
;         ||||`---- Sprite Shift
;         |||`----- Line Interrupt Enable
;         ||`------ Left Column Blank
;         |`------- Horizontal Scroll Inhibit (top 2 char rows)
;         `-------- Vertical Scroll Inhibit (right 8 char columns).

    out ($bf),a
    ld a,$80
    out ($bf),a

    ;init music
    ld hl,brahms1_start
    ld hl,demo_music_4_start
    call InitMusic1
    ld hl,brahms2_start
    call InitMusic2
    ld hl,brahms3_start
    call InitMusic3

    ;disable scrolling
    xor a
    out ($bf),a
    ld a,%10001000 ;x scroll
    out ($bf),a
    
    xor a
    out ($bf),a
    ld a,%10001001 ;y scroll
    out ($bf),a
    
    ld a,1
    ld (current_level),a
    

    ld hl,$FF0F
    ld (rocket_fuel),hl
    
    ld hl,$0000
    ld (star_color1),hl
    ld hl,$2000
    ld (star_color2),hl
    
    xor a
    ld (landing_zone_color),a
    
    ld b,0;debug mode
    call SetDifficulty
    
  game_start:
    call CutAllSound

    ;check if end of game
    ld a,(current_level)
    dec a
    cp number_of_levels
    jr nz,end_endgame_message

    
    ;draw congratulations text
    ld bc,TextCongratEnd-TextCongratStart
    ld b,c;text length in b
    ld c,2;col (tiles) in c
    ld l,5;line (tiles) in l
    ld de,TextCongratStart;text pointer in de
    call PrintTextStatic
    
    ;draw final fuel
    ld hl,(rocket_fuel)
    ld c,24;col (tiles) in c
    ld l,6;line (tiles) in l
    ld e,h;value (8bit) in e
    call PrintInt8
    
    ld a,(already_lost)
    cp 0
    jr z,+
    ;game finished, but not perfect
    ld bc,TextNotPerfectEnd-TextNotPerfectStart
    ld b,c;text length in b
    ld c,2;col (tiles) in c
    ld l,7;line (tiles) in l
    ld de,TextNotPerfectStart;text pointer in de
    call PrintTextStatic
  +:
    
    call WaitForButton
   end_endgame_message:

    ;if first level, fill fuel
    ld a,(current_level)
    cp 1
    jr nz,+
    ld hl,$FF0F
    ld (rocket_fuel),hl
    xor a
    ld (already_lost),a
  +:
  
    ;disable interrupts and turn off screen before writing to it. Will it fix tiles issues?
    di  ; disable interrupts
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
    
    ;==============================================================
    ; Clear VRAM
    ;==============================================================
    ; 1. Set VRAM write address to 0 by outputting $4000 ORed with $0000
    ld a,$00
    out ($bf),a
    ld a,$40
    out ($bf),a
    ; 2. Output 16KB of zeroes
    ld bc, $4000    ; Counter for 16KB of VRAM
    -:
        ld a,$00    ; Value to write
        out ($be),a ; Output to VRAM address, which is auto-incremented after each write
        dec bc
        ld a,b
        or c
        jp nz,-
    ld hl,0
    ld (tiles_vram_used),hl

    ;load default palette
    ld hl,Palette0Start
    ;==============================================================
    ; Load palette
    ;==============================================================
    ; 1. Set VRAM write address to CRAM (palette) address 0 (for palette index 0)
    ; by outputting $c000 ORed with $0000
    ld a,$00
    out ($bf),a
    ld a,$c0
    out ($bf),a
    ; 2. Output colour data
    ;ld hl,Palette0Start
    ld b,(Palette0End-Palette0Start)
    ld c,$be
    otir

    ;==============================================================
    ; Load font tiles
    ;==============================================================
    ; 1. Set VRAM write address to tile index 0
    ; by outputting $4000 ORed with $0000
    ld a,$00
    out ($bf),a
    ld a,$40
    out ($bf),a
    ; 2. Output tile data
    ld bc,FontData_End-FontData_Start  ; Counter for number of bytes to write
    ld hl,FontData_Start              ; Location of tile data
    
    -:
        ; Output data byte then three zeroes, because our tile data is 1 bit
        ; and must be increased to 4 bit
        ld a,(hl)        ; Get data byte
        out ($be),a
        xor a
        out ($be),a
        out ($be),a
        out ($be),a
        inc hl           ; Add one to hl so it points to the next data byte
        dec bc
        ld a,b
        or c
        jp nz,-
    ld hl,(tiles_vram_used)
    ld bc,(FontData_End-FontData_Start)*4 ; *4 because 1bpp
    add hl,bc
    ld (tiles_vram_used),hl
    
    ;==============================================================
    ; Load sprites tiles
    ;==============================================================
    ; 1. Set VRAM write address to tile index 0
    ; by outputting $4000 ORed with (tiles_vram_used)
    ld a,l
    out ($bf),a
    ld a,h
    add a,$40
    out ($bf),a
    ; 2. Output tile data
    ld bc,Tiles_sprites_End-Tiles_sprites_Start  ; Counter for number of bytes to write
    ld hl,Tiles_sprites_Start              ; Location of tile data
    
    -:
        ld a,(hl)        ; Get data byte
        out ($be),a
        inc hl           ; Add one to hl so it points to the next data byte
        dec bc
        ld a,b
        or c
        jp nz,-
    ld hl,(tiles_vram_used)
    ld bc,Tiles_sprites_End-Tiles_sprites_Start
    add hl,bc
    ld (tiles_vram_used),hl
    
    ;draw texts before turning on screen to avoid problems
    ;draw hello text
    ld bc,TextHelloEnd-TextHelloStart
    ld b,c;text length in b
    ld c,2;col (tiles) in c
    ld l,6;line (tiles) in l
    ld de,TextHelloStart;text pointer in de
    call PrintText
    
    ;draw level text
    ld bc,TextLevelEnd-TextLevelStart
    ld b,c;text length in b
    ld c,7;col (tiles) in c
    ld l,10;line (tiles) in l
    ld de,TextLevelStart;text pointer in de
    ld a,1;have to write slowly
    call PrintText
    
    ;draw level number text
    ld c,22;col (tiles) in c
    ld l,10;line (tiles) in l
    ld a,(current_level)
    ld e,a;value (8bit) in e
    call PrintInt8
    
    ; Turn screen on
    ld a,%11100000
;          |||| |`- Zoomed sprites -> 16x16 pixels
;          |||| `-- Doubled sprites -> 2 tiles per sprite, 8x16
;          |||`---- 30 row/240 line mode
;          ||`----- 28 row/224 line mode
;          |`------ VBlank interrupts
;          `------- Enable display
    out ($bf),a
    ld a,$81
    out ($bf),a

    ei  ; enable interrupts
    
    call WaitForButton



    ;disable interrupts and turn off screen before writing to it. Will it fix tiles issues?
    di  ; disable interrupts
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
    
    call SetupLevel1
    
    
    ; Turn screen on
    ld a,%11100000
;          |||| |`- Zoomed sprites -> 16x16 pixels
;          |||| `-- Doubled sprites -> 2 tiles per sprite, 8x16
;          |||`---- 30 row/240 line mode
;          ||`----- 28 row/224 line mode
;          |`------ VBlank interrupts
;          `------- Enable display
    out ($bf),a
    ld a,$81
    out ($bf),a

    ei  ; enable interrupts    
    
    ;variables initialization
    ld hl,$0
    ld (speedX),hl
    ld hl,$0
    ld (speedY),hl
    ld hl,$8000
    ld (posX),hl
    ld hl,$1400
    ld (posY),hl
    xor a
    ld (rocket_status),a

    xor a
    ld (goto_level),a
    
    ld a,1
    ld (new_frame),a

    ei;enable interruption (for vblank)    


    ;set game status to level start
    xor a
    ld (gameStatus),a
    ld hl,0
    ld (gameStatusCnt),hl


  MainLoop:

    ld a,(PauseFlag)
    cp 1
    jr z,MainLoop ;if pause do nothing in main loop
    
    ;cut noise channel sound
    ld c,%01100000;channel in c*%100000(max 3*%100000)
    call CutOneChannel
    
    call GameStatusUpdate
    
    ld hl,(WaitForVBlankFunction)
    call CallFunctionHL;act as "call (hl)"
    
    
    call UpdatePalette
    call UpdateScreen
    ;call PSGMOD_Play
    
    ;check if level finished
    ld a,(goto_level)
    or a
    jp z,MainLoop
      ;we have to change the level
      ld c,a
      call WaitForButton
      ;if just lost and pushed button 2, restart game
      ld a,b;get pushed button
      cp 2 ;if not button 2, replay level
      jr nz,+
      ;also check if fuel max (=you just lost)
      ld hl,(rocket_fuel)
      ld a,h
      cp $FF
      jr nz,+
      ld c,1;restart game to level 1
     +:
      ld a,c
      ld (current_level),a
      jp game_start
    
    
    jp MainLoop
ret

WaitForButton:
    ;out: b for button (1 or 2)
    call CutAllSound
    push af
      -:in a,($dc)
        and %00110000 ;is button 1?
        ld b,1
        cp  %00100000
        jr z,+
        in a,($dc)
        ld b,2
        and %00110000 ;is button 2?
        cp  %00010000
        jr z,+
        jr -
      +:
        ; Button down, wait for it to go up
      -:in a,($dc)
        and %00110000
        cp  %00110000
        jr nz,-
    pop af
    ret

ReadButtonsPlay:
    push af
        in a,($dc)
        and %00010000
        cp  %00000000
        jr nz,+
        call OnButton1
    +:  
        in a,($dc)
        and %00100000
        cp  %00000000
        jr nz,+
        call OnButton2
    +:  
        in a,($dc)
        and %00000001
        cp  %00000000
        jr nz,+
        call OnButtonUp
    +:  
        in a,($dc)
        and %00000010
        cp  %00000000
        jr nz,+
        call OnButtonDown
    +:  
        in a,($dc)
        and %00000100
        cp  %00000000
        jr nz,+
        call OnButtonLeft
    +:  
        in a,($dc)
        and %00001000
        cp  %00000000
        jr nz,+
        call OnButtonRight
    +:
    pop af
ret
 
OnButton1:
    ret
OnButton2:
    ret
OnButtonUp:
    ret
OnButtonDown:
    push af
    push bc
    push de
    push hl
        ld hl,(rocket_fuel) ;compare rocket_fuel to 0...
        ld a,h
        cp 0
        jp z,+
        ;if not, decrease rocket_fuel
        ld bc,(fuel_use)
        add hl,bc
        ld (rocket_fuel),hl
        

        ld bc,-$4 ;//remove 4pix/256frame to y speed
        ld hl,(speedY)
        add hl,bc
        ld (speedY),hl
        ;draw fire sprite
        ld bc,(posX)
        ld a,b
        add a,$4;x+4
        ld h,a;x in h
        ld bc,(posY)
        ld a,b
        add a,$12;y+$12
        ld l,a;y in l
        ld d,fire_tile_number+1;number of the tile in VRAM in d
        call SpriteSet8x16
        
        ;change rocket status to bottom fire (1)
        ld a,1
        ld (rocket_status),a
        
        ;noise!
        ld c,%01100000;channel in c*%100000(max 3*%100000)
        call EnableChannel
        ld a,%00001000
        call PlayNoise
        
    +:
    pop hl
    pop de
    pop bc
    pop af
ret
OnButtonLeft:
    push af
    push bc
    push de
    push hl 
        ld hl,(rocket_fuel) ;compare rocket_fuel to 0...
        ld a,h
        cp 0
        jp z,+
        ;if not, decrease rocket_fuel
        ld bc,(fuel_use)
        add hl,bc
        ld (rocket_fuel),hl
        

        ld bc,$0004 ;//add 4pix/256frame to x speed
        ld hl,(speedX)
        add hl,bc
        ld (speedX),hl
        ;draw fire sprite
        ld bc,(posX)
        ld a,b
        sub $04;x-4
        ld h,a;x in h
        ld bc,(posY)
        ld a,b
        add a,$08;y+8
        ld l,a;y in l
        ld d,fire_tile_number;number of the tile in VRAM in d
        call SpriteSet8x8
        
        ;noise!
        ld c,%01100000;channel in c*%100000(max 3*%100000)
        call EnableChannel
        ld a,%00001000
        call PlayNoise
        
    +:
    pop hl
    pop de
    pop bc
    pop af
ret
OnButtonRight:
    push af
    push bc
    push de
    push hl
        ld hl,(rocket_fuel) ;compare rocket_fuel to 0...
        ld a,h
        cp 0
        jp z,+
        ;if not, decrease rocket_fuel
        ld bc,(fuel_use)
        add hl,bc
        ld (rocket_fuel),hl
        

        ld bc,$-4 ;//remove 4pix/256frame to x speed
        ld hl,(speedX)
        add hl,bc
        ld (speedX),hl
        ;draw fire sprite
        ld bc,(posX)
        ld a,b
        add a,$0C;x+12
        ld h,a;x in h
        ld bc,(posY)
        ld a,b
        add a,$08;y+8
        ld l,a;y in l
        ld d,fire_tile_number+3;number of the tile in VRAM in d
        call SpriteSet8x8
        
        ;noise!
        ld c,%01100000;channel in c*%100000(max 3*%100000)
        call EnableChannel
        ld a,%00001000
        call PlayNoise
        
    +:
    pop hl
    pop de
    pop bc
    pop af
ret

;----------------- end debug

WaitForVBlankLava:
    push af
    push bc
    
      ;compute lava y in screen
      ld hl,(YLava)
      ld bc,(YScrollTotal)
      ;want (YLava)-(YScrollTotal)
      or a ; reset carry flag (see http://www.smspower.org/Development/Z80ProgrammingTechniques#SubHlde)
      sbc hl,bc
      ;ld a,b
      ;neg
      ;ld b,a
      ;ld a,c
      ;neg
      ;ld c,a
      ;add hl,bc ;hl=(YLava)-(YScrollTotal)
      
      ;put the 3 lava lines in screen in b,d,e 
      ld b,l
      ld a,b
      add a,$10
      jr nc,+
      ld d,$FF ;out of screen
      ld e,$FF ;out of screen
    +:
      ld d,a
      add a,$2
      jr nc,+
      ld e,$FF ;out of screen
    +:
      ld e,a
      
      ld a,h;test if hl in screen (<256)
      cp 0
      jr nz,+
      ld b,$C0 ;out of screen
      ld d,$C0 ;out of screen
      ld e,$C0 ;out of screen
    +:
      
    
    -:
      ;check for lava level:
      ;read scan line counter
      in a,($7E)
      ld c,a
      cp b;if == b, change palette
      jr nz,+
      ;==============================================================
      ; Update palette
      ;==============================================================
      ; 1. Set VRAM write address to CRAM (palette) address 0 (for palette index 0)
      ; by outputting $c000 ORed with $0006 (number of the color to change)
      ld a,$06
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$20
      out ($be),a
      jr -
    +:
      ld a,c ;get line counter back
      cp d
      jr nz,+
      ;==============================================================
      ; Update palette
      ;==============================================================
      ; 1. Set VRAM write address to CRAM (palette) address 0 (for palette index 0)
      ; by outputting $c000 ORed with $0006 (number of the color to change)
      ld a,$06
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$03
      out ($be),a
      ;change color 9
      ld a,$09
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$03
      out ($be),a
      ;change color A
      ld a,$0A
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$03
      out ($be),a
      jr -
    +:    
      ld a,c ;get line counter back
      cp e
      jr nz,+
      ;==============================================================
      ; Update palette
      ;==============================================================
      ; 1. Set VRAM write address to CRAM (palette) address 0 (for palette index 0)
      ; by outputting $c000 ORed with $0006 (number of the color to change)
      ld a,$06
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$01
      out ($be),a
      ;change color 9
      ld a,$09
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$01
      out ($be),a
      ;change color A
      ld a,$0A
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$01
      out ($be),a
      jr -
    +:    
      ;check if end of frame
      ld a,(new_frame)
      cp 0
      jr z,-

      xor a
      ld (new_frame),a
      
      
      ;get back to original color...
      ;==============================================================
      ; Update palette
      ;==============================================================
      ; 1. Set VRAM write address to CRAM (palette) address 0 (for palette index 0)
      ; by outputting $c000 ORed with $0006 (number of the color to change)
      ld a,$06
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$10
      out ($be),a
      ;change color 9
      ld a,$09
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$3a
      out ($be),a
      ;change color A
      ld a,$0A
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$00
      out ($be),a
    pop bc
    pop af
  
  ret


WaitForVBlankSimple:
    push af
    
    -:
      ;check if end of frame
      ld a,(new_frame)
      cp 0
      jr z,-

      xor a
      ld (new_frame),a
      
    pop af
  ret


PSGMOD_Play:
    ;~ ld c,0;channel in c*%100000(max 3*%100000)
    ;~ ld hl,(posY) ;Tone in hl (max 1024)
    ;~ ;ld l,h
    ;~ ;ld h,%00000011
    ;~ 
    ;~ ld a,h
    ;~ ;neg
    ;~ ld l,a
    ;~ ld h,%00000010
    ;~ 
    ;~ 
    ;~ call PlayTone
    ;~ 
    
    ;play harmonics or not depending on level number
    ;ld a,(current_level)
    ;and %00000001
    ;jr z,+
    ;call PlayMusicH
    ;ret
    ;+:
    call PlayMusic1
    ;call PlayMusic2
    ;call PlayMusic3

    ret
    
UpdatePalette:
   push af
   push bc
   push hl
    ;update star color
    ld hl,(star_color1)
    ld bc,$100 ;color change speed
    add hl,bc
    ld (star_color1),hl
    ld hl,(star_color2)
    ld bc,$100 ;color change speed
    add hl,bc
    ld (star_color2),hl
    ;==============================================================
    ; Update palette
    ;==============================================================
    ; 1. Set VRAM write address to CRAM (palette) address 0 (for palette index 0)
    ; by outputting $c000 ORed with number of the color to change
    ld a,$0F
    out ($bf),a
    ld a,$c0
    out ($bf),a
    ; 2. Output colour data
    ld hl,(star_color1)
    ld a,h
    and %00110000 ;use only bright colors (let only blue byte change)
    or  %00001111 ; R and G are at max
    out ($be),a
    
    ;update fire color 1
    ld a,$1A
    out ($bf),a
    ld a,$c0
    out ($bf),a
    ; 2. Output colour data
    ld hl,(star_color1)
    ld a,h
    and %00110101 ;use only bright colors (let only blue byte change)
    or  %00001010 ; R and G are at max
    out ($be),a
    
    ;update fire color 2
    ld a,$1B
    out ($bf),a
    ld a,$c0
    out ($bf),a
    ; 2. Output colour data
    ld hl,(star_color2)
    ld a,h
    and %00110101 ;use only bright colors (let only blue byte change)
    or  %00001010 ; R and G are at max
    out ($be),a
    
    ;if level>1, landing zone color loop
    ld a,(current_level)
    cp 1
    jp z,UpdatePalette_end
    
    ;-----------------------------landing zone part-----------------------------
    ;loop on landing_zone_color, to change colors 11-14 of palette
    ;looking at values in LandingColorLoop
    
    ;inc landing_zone_color
    ld hl,(landing_zone_color)
    ld bc,$20 ;color loop speed
    add hl,bc
    ;test if end of loop
    ld a,h
    cp 4
    jr nz,+
    ld hl,0
  +:
    ld (landing_zone_color),hl
    ; landing_zone_color is updated
    
    
    ld bc,(landing_zone_color)
    ld a,b
    ld hl,LandingColorLoop
    ;inc hl while a>0
  -:
    cp 0
    jr z,+
    inc hl
    dec a
    jr -
  +: ;hl is the correct starting address
    
    ; 1. Set VRAM write address to CRAM (palette) address 0 (for palette index 0)
    ; by outputting $c000 ORed with number of the color to change
    ld a,11 ;change color 11
    out ($bf),a
    ld a,$c0
    out ($bf),a
    ; 2. Output colour data
    ld a,(hl)
    out ($be),a
    
    inc hl
    inc b
    ld a,b
    cp 4
    jr nz,+
    ;end of the color array, loop
    ld hl,LandingColorLoop    
  +:
    ld a,12 ;change color 12
    out ($bf),a
    ld a,$c0
    out ($bf),a
    ; 2. Output colour data
    ld a,(hl)
    out ($be),a
    
    inc hl
    inc b
    ld a,b
    cp 4
    jr nz,+
    ;end of the color array
    ld hl,LandingColorLoop    
  +:
    ld a,13 ;change color 13
    out ($bf),a
    ld a,$c0
    out ($bf),a
    ; 2. Output colour data
    ld a,(hl)
    out ($be),a    

    inc hl
    inc b
    ld a,b
    cp 4
    jr nz,+
    ;end of the color array
    ld hl,LandingColorLoop    
  +:
    ld a,14 ;change color 14
    out ($bf),a
    ld a,$c0
    out ($bf),a
    ; 2. Output colour data
    ld a,(hl)
    out ($be),a   
    
  UpdatePalette_end:
   pop hl
   pop bc
   pop af    
   ret 

UpdateScreen:
    ret


DoGameLogicDbg:
    push af
  
    push bc
    push de
    push hl
    
  
    call ResetSprites
    ;TODO: make it better
    ;update rocket status
    ;if 1, set to 0
    ld a,(rocket_status)
    cp 1
    jr nz,+
    xor a
    ld (rocket_status),a
  +:
    
    ;mechanics
    ;increment Y-speed (gravity)
    ld hl,(speedY)
    inc hl
    inc hl
    ld (speedY),hl
    
    ;update x pos
    ld bc,(posX)
    ld hl, (speedX)
    add hl,bc
    ld (posX),hl
    ;update y pos
    ld bc,(posY)
    ld hl, (speedY)
    add hl,bc
    ld (posY),hl
    
    ;y scroll depending on posY
    ;if posY<60, scroll up
    ;if posY>100, scroll down
    ld hl, (posY)
    ld a,h ;a is the entire part of posY
    cp 60
    jr nc,+
    ld c,-1
    call VScrollManager
    ;once again to have a 2-speed scrolling
    ld hl, (posY)
    ld a,h ;a is the entire part of posY
    cp 60
    jr nc,+
    ld c,-1
    call VScrollManager
  +:
    cp 100
    jr c,+
    ld c,1
    call VScrollManager
    ld hl, (posY)
    ld a,h ;a is the entire part of posY
    cp 100
    jr c,+
    ld c,1
    call VScrollManager
  +:
    
    
    ;set speed to 0
    ;ld hl,0
    ;ld (speedX),hl
    ;ld (speedY),hl
    
    call ReadButtonsPlay
    

    
    ;draw rocket sprites
    ld bc,(posX)    
    ld h,b;x in h
    ld bc,(posY)    
    ld l,b;y in l
    call DrawRocket
    
    ;draw fuel logo
    ld h,8 ;col (pix) in h
    ld l,8 ;line (pix) in l
    ld d,fuel_tile_number
    call SpriteSet8x8
    ;fuel value
    ld hl,(rocket_fuel)
    ld e,h;value (8bit) in e
    ld h,16 ;col (pix) in h
    ld l,8 ;line (pix) in l
    call PrintInt10Sprite
    
    call drawWarning
    call TestAllCollisions

    ;hide other sprites
    call SetLastSprite
    
    pop hl
    pop de
    pop bc
    pop af

    ret


;check if speed is too big, write a warning
drawWarning:
    push af
    push bc
    push hl


    ;in x:
    ld hl,(speedX)
    ;test if speed >0
    ld a,h
    cp $80
    jr c,drawWarning_hz_pos
    
    ;here speedX<0
    cp 255
    jr z,+
    jp drawWarning_right
  +: ;has to test tolerance
    ;test if l>speed_neg_tolerance
    ld bc,(speed_neg_tolerance)
    ld a,l
    cp c
    jr c,drawWarning_right
    jp drawWarning_test_Y
  
  drawWarning_hz_pos:
    ;here speedX>0
    ld a,h
    cp 0
    jr z,+
    jp drawWarning_left
  +: ;has to test tolerance
    ;test if l<speed_pos_tolerance
    ld bc,(speed_pos_tolerance)
    ld a,l
    cp c
    jr nc,drawWarning_left
    jp drawWarning_test_Y
    
  drawWarning_left:
    ;draw bubble left
    ld bc,(posX)
    ld a,b
    add a,10
    ld h,a;col (pix) in h
    ld bc,(posY)
    ld a,b
    add a,-12
    ld l,a;line (pix) in l
    ld d,bubble_right_tile_number
    call SpriteSet16x8
    ;draw bubble bottom
    ld bc,(posX)
    ld a,b
    add a,10
    ld h,a;col (pix) in h
    ld bc,(posY)
    ld a,b
    add a,-4
    ld l,a;line (pix) in l
    ld d,bubble_bottom_tile_number
    call SpriteSet16x8
    jp drawWarning_test_Y
  drawWarning_right:
    ;draw bubble left
    ld bc,(posX)
    ld a,b
    add a,10
    ld h,a;col (pix) in h
    ld bc,(posY)
    ld a,b
    add a,-12
    ld l,a;line (pix) in l
    ld d,bubble_left_tile_number
    call SpriteSet16x8
    ;draw bubble bottom
    ld bc,(posX)
    ld a,b
    add a,10
    ld h,a;col (pix) in h
    ld bc,(posY)
    ld a,b
    add a,-4
    ld l,a;line (pix) in l
    ld d,bubble_bottom_tile_number
    call SpriteSet16x8
    
  drawWarning_test_Y:
    ;Test in y:
    ld hl,(speedY)
    ;test if speed positive (h<$80)
    ld a,h
    cp $80
    jr nc,drawWarning_end
    
    ;test if h==0
    cp 0
    jr nz,drawWarning_down
    ;then test if l<speed_pos_tolerance
    ld bc,(speed_pos_tolerance)
    ld a,l
    cp c
    jr nc,drawWarning_down
    jr drawWarning_end
    
    drawWarning_down:
      ;draw bubble top 
      ld bc,(posX)
      ld a,b
      add a,14
      ld h,a;col (pix) in h
      ld bc,(posY)
      ld a,b
      add a,-4
      ld l,a;line (pix) in l
      ld d,bubble_down_tile_number
      call SpriteSet16x8
    
    drawWarning_bubble_bottom:
      ;draw bubble bottom
      ld bc,(posX)
      ld a,b
      add a,14
      ld h,a;col (pix) in h
      ld bc,(posY)
      ld a,b
      add a,4
      ld l,a;line (pix) in l
      ld d,bubble_bottom_tile_number
      call SpriteSet16x8
    
    drawWarning_end:
    pop hl
    pop bc
    pop af
    ret

;convert posX,posY (in pixel) and deltaX deltaY (in pixels)
;to number of tiles in VRAM (with scrolling)
;posX MSB in b
;posY MSB in c
;deltaX in d
;deltaY in e
;output:
;  x in h (in tiles)
;  y in l (in tiles)
RocketPos2vramTiles:
  push af
  push bc
    push bc;save (posX,posY)
    
    ;X part
    ld h,0
    ld l,b ;get posX
    ld c,d
    ld b,0 ;deltaX in bc
    add hl,bc
    
    ld a,(XScrollVDP)
    ld c,a
    add hl,bc;add scrolling value
    
    ld h,l
    srl h;divide by 8
    srl h
    srl h
    ;x in h (in tiles)
    
    ;Y part
    pop bc ;get (posX,posY)
    push hl ;record x for later
    
    ld l,c
    ld h,0;Y in hl
    ld c,e
    ld b,0 ;deltaY in bc
    add hl,bc
    ;add scrolling value
    ld a,(YScrollVDP)
    ld c,a
    ld b,0
    add hl,bc
    ;have to check if hl>=224
    ;first test if >255
    ld a,h
    cp 0
    jr nz,RocketPos2vramTiles_Vcarry ;if>255, have a carry
    ld a,l
    cp 224
    jr c,RocketPos2vramTiles_VcarryOK;if<224, no carry
  RocketPos2vramTiles_Vcarry:
    ld bc,-224
    add hl,bc
    
    ;carry is handled
  RocketPos2vramTiles_VcarryOK:
    srl l ;divide by 8
    srl l
    srl l
    ;y in l (in tiles)
    
    ;get x back
    pop bc
    ld h,b
    
  pop bc
  pop af
  ret

TestAllCollisions:
      ;test point posX+8,posY
      ld bc,(posX);posX in b
      ld hl,(posY)
      ld c,h;posY in c
      ld d,8;deltaX in d
      ld e,0;deltaY in e
      ;if small rocket, add 2 pixels in deltaY
      ld a,(big_rocket)
      cp 1
      jr z,+
      inc e
      inc e
    +:
      call RocketPos2vramTiles
;      call TestCollision
      
      ;test landing point posX+8,posY+25
      ld d,8;deltaX in d
      ld e,25;deltaY in e
      call RocketPos2vramTiles
      call TestLanding
      
      ;test point posX+2,posY+24
      ld d,2;deltaX in d
      ld e,24;deltaY in e
      call RocketPos2vramTiles
;      call TestCollision
      
      ;test point posX+8,posY+24
      ld d,8;deltaX in d
      ld e,24;deltaY in e
      call RocketPos2vramTiles
;      call TestCollision
      
      ;test point posX+13,posY+24
      ld d,13;deltaX in d
      ld e,24;deltaY in e
      call RocketPos2vramTiles
;      call TestCollision
      
      ;test point posX+4,posY+12
      ld d,4;deltaX in d
      ld e,12;deltaY in e
      call RocketPos2vramTiles
;      call TestCollision
      
      ;test point posX+11,posY+12
      ld d,11;deltaX in d
      ld e,12;deltaY in e
      call RocketPos2vramTiles
;      call TestCollision
      
    ret

;posX MSB in b ;TODO: use bc to get mod 8 part of positions
;posY MSB in c
;x in h (in tiles)
;y in l (in tiles)
TestCollision:;use tilemap collision data
    push af
    push bc
    push hl
    
      ;compute tile number
      ld b,0
      ld c,h;x in bc
      call Multby32 ;hl=l*32
      add hl,bc
      add hl,hl;multiply by 2
      ld b,l
      ld c,h
      ;tile number in bc
      
      ;try to read data in vram
      ; 1. Set VRAM read address to name table index 0
      ; by outputting $0000 ORed with $3800+bc
      ld a,b
      out ($bf),a
      ld a,$38
      add a,c
      out ($bf),a
      ; 2. get tilemap data
      in a,($be)
      ld h,a
      in a,($be)
      ld l,a
      
      ;collision data is in l & %11100000
      
      ld a,l
      and %11100000
      cp %00100000
      jr nz,+
      ;touches=> explode
      ld a,2
      ld (rocket_status),a
      ld hl,0
      ld (gameStatusCnt),hl
      ld a,2 ;lost
      ld (gameStatus),a
    +:
      
      
      
    TestCollision_end:
    pop hl
    pop bc
    pop af
    ret


;posX MSB in b ;TODO: use bc to get mod 8 part of positions
;posY MSB in c
;x in h (in tiles)
;y in l (in tiles)
TestLanding:;use tilemap collision data
    push af
    push bc
    push hl
    
      ;compute tile number
      ld b,0
      ld c,h;x in bc
      call Multby32 ;hl=l*32
      add hl,bc
      add hl,hl;multiply by 2
      ld b,l
      ld c,h
      ;tile number in bc
      
      ;try to read data in vram
      ; 1. Set VRAM read address to name table index 0
      ; by outputting $0000 ORed with $3800+bc
      ld a,b
      out ($bf),a
      ld a,$38
      add a,c
      out ($bf),a
      ; 2. get tilemap data
      in a,($be)
      ld h,a
      in a,($be)
      ld l,a
      
      ;collision data is in l & %11100000
      
      ld a,l
      and %11100000
      cp %11100000
      jr nz,TestLanding_end
      
      TestLanding_debug:
        ;touches=> test if speed is low enough
        ;in x:
        ld hl,(speedX)
        ;test if speed >0
        ld a,h
        cp 0
        jr z,+
        cp 255
        jr z,+
        jp TestLanding_explode ;if h!=0 and h!=255, explode
      +:
        ;first, test if h==0
        cp 0
        jr nz,+
        ;positive hz speed
        ;test if l<speed_pos_tolerance
        ld bc,(speed_pos_tolerance)
        ld a,l
        cp c
        jr nc,TestLanding_explode
        jp TestLanding_y
       +:
        ;negative hz speed
        ;test if l>speed_neg_tolerance
        ld bc,(speed_neg_tolerance)
        ld a,l
        cp c
        jr c,TestLanding_explode ;TODO: speed_neg_tolerance must be -(speed_pos_tolerance)-1?
        
      TestLanding_y:  
        ;in y:
        ld hl,(speedY)
        ;first, test if h==0
        ld a,h
        cp 0
        jr nz,TestLanding_explode
        ;then test if l<speed_pos_tolerance
        ld bc,(speed_pos_tolerance)
        ld a,l
        cp c
        jr nc,TestLanding_explode

      ;win
      ld hl,0
      ld (gameStatusCnt),hl
      ld a,3 ;won
      ld (gameStatus),a
      jr TestLanding_end
      
    TestLanding_explode:
      ld a,2
      ld (rocket_status),a
      ld hl,0
      ld (gameStatusCnt),hl
      ld a,2 ;lost
      ld (gameStatus),a
      
      ;refill!
      ld hl,$FF0F
      ld (rocket_fuel),hl
      ld a,1
      ld (already_lost),a
      
    TestLanding_end:
    pop hl
    pop bc
    pop af
    ret


.ends ;--------------- end game section





