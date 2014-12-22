
.ramsection "Game variables" slot 2
  YScrollTotal             dw    ; total Y scroll in level (in pixels)
  YScrollTotalMax          dw    ; maximum of YScrollTotal value (in pixels)
  YScrollVDP               db    ; VDP YScroll (in pixels)
  LevelMapStart            dw    ; pointer to tilemap start (in bytes)
  NextMapDataStart         dw    ; pointer to tilemap new part to load (in bytes)
  
  YLava                    dw    ; total Y lava in level (in pixels)
.ends


.section "game functions" free

SetupLevel1:
    ld hl,Palette1Start
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
    ;ld hl,Palette1Start
    ld b,(Palette1End-Palette1Start)
    ld c,$be
    otir

    ;load tilemap of current level
    ld hl,Tilemap1Start
    ;hl is where the tilemap of the level starts
    ;==============================================================
    ; Write tilemap data
    ;==============================================================
    ; 1. Set VRAM write address to name table index 0
    ; by outputting $4000 ORed with $3800+0
    ld a,$00
    out ($bf),a
    ld a,$38|$40
    out ($bf),a
    ; 2. Output tilemap data
    ;ld hl,Tilemap1Start
    ld bc,$600+$40; Counter for number of bytes to write
    -:
        ld a,(hl)    ; Get data byte
        out ($be),a
        inc hl       ; Point to next tile
        dec bc
        ld a,b
        or c
        jr nz,-
    
    ;initialize scroll variables
    ld hl,Tilemap1Start
    ld (LevelMapStart),hl
    ld a,0
    ld (YScrollVDP),a
    ld hl,0
    ld (YScrollTotal),hl
    
    ;compute max y scroll value
    ld hl,((Tilemap1End-Tilemap1Start)/64-24)*8
    ld (YScrollTotalMax),hl
    
    ld hl,500
    ld (YLava),hl
    
ret

;update v scroll
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
    
    ;update YScrollVDP
    ld a,(YScrollVDP)
    inc a
    ;check if YScrollVDP >223
    cp 224
    jr c,+
    ;if YScrollVDP >223
    ld a,0

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
    jp z,VScrollManager_end
  +:
    ;update YScrollTotal
    dec hl
    ld (YScrollTotal),hl
    
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
    ld a,0
    out ($bf),a
    ld a,%10001000
    out ($bf),a

    ;game init
    ld a,1
    ld (current_level),a
    


    ld hl,$FF0F
    ld (rocket_fuel),hl
    
    ld hl,$0000
    ld (star_color),hl

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
    call PrintInt
    
    ld a,1
    ld (current_level),a
    
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
    ld a,0
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
    ; Load tiles
    ;==============================================================
    ; 1. Set VRAM write address to tile index 0
    ; by outputting $4000 ORed with $0000
    ld a,$00
    out ($bf),a
    ld a,$40
    out ($bf),a
    ; 2. Output tile data
    ld hl,TilesStart              ; Location of tile data
    ld bc,TilesEnd-TilesStart  ; Counter for number of bytes to write
    -:
        ; Output data byte then three zeroes, because our tile data is 1 bit
        ; and must be increased to 4 bit
        ld a,(hl)        ; Get data byte
        out ($be),a
        inc hl           ; Add one to hl so it points to the next data byte
        dec bc
        ld a,b
        or c
        jp nz,-

    ;draw texts befor turning on screen to avoid problems
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
    call PrintInt
    
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

    ld a,0
    ld (goto_level),a
    
    ld a,1
    ld (new_frame),a

    ei;enable interruption (for vblank)


    ;~ ;test sound
    ;~ ld c,0;channel in c*%100000(max 3*%100000)
    ;~ call EnableChannel
    ;~ 
    ;~ ld c,%01100000;channel in c*%100000(max 3*%100000)
    ;~ call EnableChannel

  MainLoop:

    ld a,(PauseFlag)
    cp 1
    jr z,MainLoop ;if pause do nothing in main loop
    
    ;cut noise channel sound
    ld c,%01100000;channel in c*%100000(max 3*%100000)
    call CutOneChannel

    call DoGameLogicDbg
    call WaitForVBlank
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

ReadButtons:
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
        call OnButtonUpDbg
    +:  
        in a,($dc)
        and %00000010
        cp  %00000000
        jr nz,+
        call OnButtonDownDbg
    +:  
        in a,($dc)
        and %00000100
        cp  %00000000
        jr nz,+
        call OnButtonLeftDbg
    +:  
        in a,($dc)
        and %00001000
        cp  %00000000
        jr nz,+
        call OnButtonRightDbg
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
        ld bc,fuel_use
        add hl,bc
        ld (rocket_fuel),hl
        
        
        ld a,(number_of_sprites)
        ld e,a;sprite index in e
        inc a
        ld (number_of_sprites),a

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
        add a,$18;y+24
        ld l,a;y in l
        ld d,fire_tile_number+1;number of the tile in VRAM in d
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
        ld bc,fuel_use
        add hl,bc
        ld (rocket_fuel),hl
        
        ld a,(number_of_sprites)
        ld e,a;sprite index in e
        inc a
        ld (number_of_sprites),a

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
        ld bc,fuel_use
        add hl,bc
        ld (rocket_fuel),hl
        
        ld a,(number_of_sprites)
        ld e,a;sprite index in e
        inc a
        ld (number_of_sprites),a

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
        ld d,fire_tile_number+2;number of the tile in VRAM in d
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

;------------------ debug

OnButtonUpDbg:
    push bc
    push hl
        ld bc,$100 ;
        ld hl,(speedY)
        add hl,bc
        ld (speedY),hl
    +:
    pop hl
    pop bc
ret
OnButtonDownDbg:
    push bc
    push hl
        ld bc,-$100 ;
        ld hl,(speedY)
        add hl,bc
        ld (speedY),hl
    +:
    pop hl
    pop bc
ret
OnButtonLeftDbg:
    push bc
    push hl
        ld bc,$100 ;
        ld hl,(speedX)
        add hl,bc
        ld (speedX),hl
    +:
    pop hl
    pop bc
ret
OnButtonRightDbg:
    push bc
    push hl
        ld bc,-$100 ;
        ld hl,(speedX)
        add hl,bc
        ld (speedX),hl
    +:
    pop hl
    pop bc
ret
;----------------- end debug

WaitForVBlank:
    push af
    push bc
    
      ;compute lava y in screen
      ld hl,(YLava)
      ld bc,(YScrollTotal)
      ;want (YLava)-(YScrollTotal)
      ld a,b
      neg
      ld b,a
      ld a,c
      neg
      ld c,a
      add hl,bc ;hl=(YLava)-(YScrollTotal)
      
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
      ; by outputting $c000 ORed with $0005 (number of the color to change)
      ld a,$05
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
      ; by outputting $c000 ORed with $0005 (number of the color to change)
      ld a,$05
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$03
      out ($be),a
      ;change color 8
      ld a,$08
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$03
      out ($be),a
      ;change color 9
      ld a,$08
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
      ; by outputting $c000 ORed with $0005 (number of the color to change)
      ld a,$05
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$01
      out ($be),a
      ;change color 8
      ld a,$08
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
      jr -
    +:    
      ;check if end of frame
      ld a,(new_frame)
      cp 0
      jr z,-

      ld a,0
      ld (new_frame),a
      
      
      ;get back to original color...
      ;==============================================================
      ; Update palette
      ;==============================================================
      ; 1. Set VRAM write address to CRAM (palette) address 0 (for palette index 0)
      ; by outputting $c000 ORed with $0005 (number of the color to change)
      ld a,$05
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$10
      out ($be),a
      ;change color 8
      ld a,$08
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$3a
      out ($be),a
      ;change color 9
      ld a,$09
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ld a,$00
      out ($be),a
    pop bc
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
    ld hl,(star_color)
    ld bc,$100 ;color change speed
    add hl,bc
    ld (star_color),hl
    ;==============================================================
    ; Update palette
    ;==============================================================
    ; 1. Set VRAM write address to CRAM (palette) address 0 (for palette index 0)
    ; by outputting $c000 ORed with $000F (number of the color to change)
    ld a,$0F
    out ($bf),a
    ld a,$c0
    out ($bf),a
    ; 2. Output colour data
    ld hl,(star_color)
    ld a,h
    and %00110000 ;use only bright colors (let only blue byte change)
    or  %00001111 ; R and G are at max
    out ($be),a
    
    ;update fire color 2
    ld a,$1C
    out ($bf),a
    ld a,$c0
    out ($bf),a
    ; 2. Output colour data
    ld hl,(star_color)
    ld a,h
    and %00110101 ;use only bright colors (let only blue byte change)
    or  %00001010 ; R and G are at max
    out ($be),a
    
    
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
    
  
    ld a,$06 ; at least 6 sprites to show (rocket)
    ld (number_of_sprites),a
    
    ;mechanics
    
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
    
    ;y scroll depending on speedY
    ld hl, (speedY)
    ld c,h
    call VScrollManager
    ld hl, (YScrollTotal)
    add hl,hl
    add hl,hl
    ld (YLava),hl

    
    ;set speed to 0
    ld hl,0
    ld (speedX),hl
    ld (speedY),hl
    
    call ReadButtons ;updates number_of_sprites
    ;hide other sprites
    ld a,(number_of_sprites)
    ld c,$01 ; hide only next sprite, the following will be hidden too
    call HideSprites

    
    ;draw rocket sprites
    ld bc,(posX)    
    ld h,b;x in h
    ld bc,(posY)    
    ld l,b;y in l
    ld d,rocket_tile_number;number of the tile in VRAM in d
    ld e,$0;sprite index in e, here these are the first sprites used
    call SpriteSet16x24
    
        
    pop hl
    pop de
    pop bc
    pop af

    ret

DoGameLogic:
    push af
  
    push bc
    push de
    push hl
  
    ld a,$06 ; at least 6 sprites to show (rocket)
    ld (number_of_sprites),a
    
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
    
    call ReadButtons ;updates number_of_sprites
    ;hide other sprites
    ld a,(number_of_sprites)
    ld c,$01 ; hide only next sprite, the following will be hidden too
    call HideSprites

    
    ;draw rocket sprites
    ld bc,(posX)    
    ld h,b;x in h
    ld bc,(posY)    
    ld l,b;y in l
    ld d,rocket_tile_number;number of the tile in VRAM in d
    ld e,$0;sprite index in e, here these are the first sprites used
    call SpriteSet16x24

    ;draw texts
    ld hl,(rocket_fuel)
    ld e,h;value (8bit) in e
    ld c,1 ;col (tiles) in c
    ld l,0 ;line (tiles) in l
    call PrintInt
    
    call drawWarning
    call TestAllCollisions
        
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

        ;clear text
        ld bc,TextNoWarnEnd-TextNoWarnStart
        ld b,c;text length in b
        ld c,4;col (tiles) in c
        ld l,1;line (tiles) in l
        ld de,TextNoWarnStart;text pointer in de
        call PrintText

      ld hl,(speedX)  
      ;ld bc,speedX_tolerance
      ;add hl,bc ;hl must be < speedX_tolerance*2
      ld bc,$80
      add hl,bc ;h must be < 1
      ld a,0
      cp h
      jr z,+
        ;draw text
        ld bc,TextWarnXEnd-TextWarnXStart
        ld b,c;text length in b
        ld c,4;col (tiles) in c
        ld l,1;line (tiles) in l
        ld de,TextWarnXStart;text pointer in de
        call PrintText
     +:
      ;in y: TODO : use speedY_tolerance
      ld hl,(speedY)  
      ld bc,$80
      add hl,bc ;h must be < 1
      ld a,0
      cp h
      jr z,+
        ;draw text
        ld bc,TextWarnYEnd-TextWarnYStart
        ld b,c;text length in b
        ld c,4;col (tiles) in c
        ld l,1;line (tiles) in l
        ld de,TextWarnYStart;text pointer in de
        call PrintText
     +:
    pop hl
    pop bc
    pop af
    ret


TestAllCollisions:
      ;tests if point posX+8,posY is on a full tile
      ld bc,(posX)    
      ld h,b
      srl h
      srl h
      srl h
      inc h ;add 8 pix!
      ;x in h (in tiles)
      ld bc,(posY)    
      ld l,b
      srl l
      srl l
      srl l
      ;y in l (in tiles)
      call TestCollision
      
      ;tests if point posX+8,posY+24 is on a full tile
      ;optimization: update only l
      inc l;add 24 pix!
      inc l
      inc l
      call TestCollision
    ret

;x in h (in tiles)
;y in l (in tiles)
TestCollision:;TODO: using only level1 data!
    push af
    push bc
    push hl
    
      ;compute tile number
      ld b,0
      ld c,h;x in bc
      call Multby32
      add hl,bc
      ld b,h
      ld c,l
      ;tile number in bc
      push bc
      
      ;load tilemap of current level
      ld hl,Tilemap1Start
      ld a,(current_level)
      ld bc,level_mem_size
    -:
      dec a
      cp 0
      jr z,+
      add hl,bc
      jr -
    +:;hl is where the tilemap of the level starts
    
      pop bc
      ;ld hl,Tilemap1Start
      add hl,bc
      add hl,bc ;hl is the pointer to the tile number
      
      
      ;if (hl)>number_of_empty_tiles
      ld a, number_of_empty_tiles
      ld b,a
      ld a,(hl)
      cp b
      jp c,end_TestCollision
      ;it's not and emprty tile... check if it is a landing zone
      ld b,landing_tile_number
      cp b
      jr z,try_to_land
      ;check if it is a full tile
      ld a,(hl)
      ld b,a
      ld a, last_full_tile
      cp b
      jp c,end_TestCollision
      jr destroy
      
   try_to_land:   
      ;good place, now check speeds
      ;in x: TODO : use speedX_tolerance
      ld hl,(speedX)  
      ;ld bc,speedX_tolerance
      ;add hl,bc ;hl must be < speedX_tolerance*2
      ld bc,$80
      add hl,bc ;h must be < 1
      ld a,0
      cp h
      jr nz,destroy
      ;in y: TODO : use speedY_tolerance
      ld hl,(speedY)  
      ld bc,$80
      add hl,bc ;h must be < 1
      ld a,0
      cp h
      jr nz,destroy
      
      
      ;draw lander ; maybe draw it as a background ?
      ld bc,(posX)    
      ld h,b;x in h
      ld bc,(posY)    
      ld l,b;y in l
      ld bc,$0808
      add hl,bc ;draw the guy a little further
      ld d,guy_tile_number;number of the tile in VRAM in d
      ld e,$6;sprite index in e, 6 because we draw the rocket too
      call SpriteSet16x16
      ;next level
      ld a,(current_level)
      inc a
      ld (goto_level),a
      ;draw text
      ld bc,TextWonEnd-TextWonStart
      ld b,c;text length in b
      ld c,2;col (tiles) in c
      ld l,3;line (tiles) in l
      ld de,TextWonStart;text pointer in de
      call PrintTextStatic
      
      jp end_TestCollision
      
    destroy:
        ;return to correct yellow
        ld a,$1C
        out ($bf),a
        ld a,$c0
        out ($bf),a
        ; 2. Output colour data
        ld a,$0f
        out ($be),a
    
        ;show explosion
        ld bc,(posX)    
        ld h,b;x in h
        ld bc,(posY)    
        ld l,b;y in l
        ld d,explosion_tile_number;number of the tile in VRAM in d
        ld e,$0;sprite index in e, 0 because we replace the rocket
        call SpriteSet16x24
      
        ld a,(current_level)
        ;ld a,1 ;return to first level
        ld (goto_level),a
        
        ;refill!
        ld hl,$FF0F
        ld (rocket_fuel),hl
        ld a,1
        ld (already_lost),a
        
        ;draw text
        ld bc,TextLostEnd-TextLostStart
        ld b,c;text length in b
        ld c,2;col (tiles) in c
        ld l,3;line (tiles) in l
        ld de,TextLostStart;text pointer in de
        call PrintTextStatic
        
    end_TestCollision:
    pop hl
    pop bc
    pop af
    ret


.ends ;--------------- end game section


;==============================================================
; Data
;==============================================================




