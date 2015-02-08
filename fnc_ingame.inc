;-------------------------------------------------------------------
DrawRocket
  ;x in h
  ;y in l
  
  push af
  push bc
  push de
  
    ;select rocket image
    ld a,(rocket_status)  
    cp 0
    jr nz,DrawRocket_testFire ;if 0, normal image
    ld b,normal_rocket_tile_number
    ;test if small rocket
    ld a,(big_rocket)
    cp 0
    jr nz,+
    inc b;if small rocket start 2 tiles later
    inc b
  +: 
    jr DrawRockey_top_ready
    
  DrawRocket_testFire:
    cp 1
    jr nz,DrawRocket_testExplosion ;if 1, bottom fire image
    ld b,fire_rocket_tile_number
    ;test if small rocket
    ld a,(big_rocket)
    cp 0
    jr nz,+
    inc b;if small rocket start 2 tiles later
    inc b
  +: 
    jr DrawRockey_top_ready
    
  DrawRocket_testExplosion:
    cp 2
    jr nz,DrawRockey_top_ready ;if 2, explosion image
    ld b,explosion_tile_number    
  
  
  
  DrawRockey_top_ready:
    ;top part
    ld d,b;number of the tile in VRAM in d
    
    call SpriteSet16x8
    
    ;select rocket image
    ld a,(rocket_status)  
    cp 0
    jr nz,+ ;if 0, normal image
    ld b,normal_rocket_tile_number+4
    jr DrawRockey_bottom_ready
  +:
    cp 1
    jr nz,+ ;if 1, bottom fire image
    ld b,fire_rocket_tile_number+4
    jr DrawRockey_bottom_ready
  +:
    cp 2
    jr nz,+ ;if 2, explosion image
    ld b,explosion_tile_number+2
  +:
  
  DrawRockey_bottom_ready:
    ;bottom part
    ld a,l
    add a,8
    ld l,a;y+8
    ld d,b
    inc e
    inc e
    call SpriteSet16x16

  pop de
  pop bc
  pop af
  ret
  
;Print a text in the middle of the screen when vertical scroll
PrintTextMiddleVScroll:
    ;col (tiles) in c
    ;line relative to screen top (tiles) in l
    ;text pointer in de
    ;text length in b
    ;the constant "diff_tile_ascii"  must represent difference between index in tiles and in ascii
    ;YScrollVDP variable is used
    
    ld a,(YScrollVDP)
    srl a
    srl a
    srl a
    add a,l;add YScrollVDP/8 to line number
    
    ;if a>=28, a=a-28
    cp 27
    jr c,+
      sub 28
  +:
    ld l,a
    call PrintText

    ret


;update game status
;gameStatus= 0: start, 1: play, 2: lost, 3: won
GameStatusUpdate:
    ld hl,(gameStatusCnt)
    ld a,(gameStatus)
    cp 0 ;test on gameStatus
    jp nz,GameStatusUpdate_sup0
    
      ;if gameStatus==0 (start)
      ld bc,$12
      add hl,bc
      ld a,h
      cp 2
      jr nz,+
      ;if gameStatusCnt too big, change to play status
      ld a,1
      ld (gameStatus),a
      ld hl,0
      ld (gameStatusCnt),hl
    +:
      ;actions if gameStatus==0
      ;TODO: show direction?
      call DoGameLogicDbg
      jp GameStatusUpdate_end
      
  GameStatusUpdate_sup0:
    ld a,(gameStatus)
    cp 1 ;test on gameStatus
    jp nz,GameStatusUpdate_sup1
    ;if gameStatus==1 (play)
      call DoGameLogicDbg
      jp GameStatusUpdate_end
    
  GameStatusUpdate_sup1:
    ld a,(gameStatus)
    cp 2 ;test on gameStatus
    jp nz,GameStatusUpdate_sup2
    ;if gameStatus==2 (lost)
      call DoLost
      jp GameStatusUpdate_end
    
  GameStatusUpdate_sup2:
    ld a,(gameStatus)
    cp 3 ;test on gameStatus
    jp nz,GameStatusUpdate_end
    ;if gameStatus==3 (won)
      call DoWon
      jp GameStatusUpdate_end
  
  
  GameStatusUpdate_end:
    ret
    



;-------------------------------------------------------------------------------
DoLost:
    call ResetSprites
    
    ;read button 1
    in a,($dc)
    and %00010000
    cp  %00000000
    jr nz,+
        ld sp, $dff0 ;clear stack
        jp game_start
  +:  
    
    ;read button 2
    in a,($dc)
    and %00100000
    cp  %00000000
    jr nz,+
        ld sp, $dff0 ;clear stack
        jp RunGame
  +:
    
    ;draw rocket sprites
    ld bc,(posX)    
    ld h,b;x in h
    ld bc,(posY)    
    ld l,b;y in l
    ld e,$0;sprite index in e, here these are the first sprites used
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
    
    ;draw lost text
    ld bc,TextLostEnd-TextLostStart
    ld b,c;text length in b
    ld c,7;col (tiles) in c
    ld l,10;line (tiles) in l
    ld de,TextLostStart;text pointer in de
    ld a,1;have to write slowly ?????????
    call PrintTextMiddleVScroll

    ;hide other sprites
    call SetLastSprite
    

    ret


;-------------------------------------------------------------------------------
DoWon:
    call ResetSprites
    
    ;read button 1
    in a,($dc)
    and %00010000
    cp  %00000000
    jr nz,+
        ;ld sp, $dff0 ;clear stack
        jp game_start
  +:  
    
    
    ;draw rocket sprites
    ld bc,(posX)    
    ld h,b;x in h
    ld bc,(posY)    
    ld l,b;y in l
    ld e,$0;sprite index in e, here these are the first sprites used
    call DrawRocket
    
    ;draw player
    ld bc,(posX)    
    ld h,b;x in h
    ld bc,(posY)    
    ld l,b;y in l
    ld bc,$0808
    add hl,bc ;draw the guy a little further
    ld d,guy_tile_number;number of the tile in VRAM in d
    ld e,$6;sprite index in e, 6 because we draw the rocket too
    call SpriteSet16x16
    
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
    
    ;draw won text
    ld bc,TextWonEnd-TextWonStart
    ld b,c;text length in b
    ld c,7;col (tiles) in c
    ld l,10;line (tiles) in l
    ld de,TextWonStart;text pointer in de
    ld a,1;have to write slowly ?????????
    call PrintTextMiddleVScroll

    ;hide other sprites
    call SetLastSprite
    

    ret





