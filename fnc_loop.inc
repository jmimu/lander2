StartGame:
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

    ld hl,$FF0F
    ld (rocket_fuel),hl
    xor a
    ld (already_lost),a

NewLevel:
    call CutAllSound

    ;reset scrolling
    xor a
    ld (XScrollVDP),a
    ld (YScrollVDP),a
    ld hl,0
    ld (XScrollTotal),hl
    ld (YScrollTotal),hl
    
    xor a
    ld (XScrollVDP),a
    ;VDP hz scrolling
    out ($bf),a
    ld a,%10001000
    out ($bf),a
    
    xor a
    ld (YScrollVDP),a
    ;VDP vertical scrolling
    out ($bf),a
    ld a,%10001001
    out ($bf),a
    

    ;check if end of game
    ld a,(current_level)
    dec a
    cp number_of_levels
    jr nz,+
    call endGame
   +:
    
    call showLevel
    
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
    
    call SelectLevel
    
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
    
    ld a,1
    ld (new_frame),a

    ei;enable interruption (for vblank)    

    ;check if fuel reset
    ld a,(refuel)
    cp 0
    jr z,+
      ld hl,$FF0F
      ld (rocket_fuel),hl
   +: 

    
    ;set game status to level start
    ld a,1
    ld (gameStatus),a
    ld hl,0
    ld (gameStatusCnt),hl
    
GameLoop:
    ld hl,(WaitForVBlankFunction)
    call CallFunctionHL;act as "call (hl)"

    ld a,(PauseFlag)
    cp 1
    jp z,GameLoop ;if pause do nothing in main loop
    
    ;cut noise channel sound
    ld c,%01100000;channel in c*%100000(max 3*%100000)
    call CutOneChannel
    
    call GameStatusUpdate
    
    
    
    
    call UpdatePalette
    call UpdateScreen
    ;call PSGMOD_Play
    
    jp GameLoop


;-----------------------------------------------------------------------
SelectLevel:
    ;TODO: add end of game check and show level here?
    ld a,(current_level)
    cp 1
    jr nz,+
      call SetupLevel12
      jp SelectLevel_end
  +:
    cp 2
    jr nz,+
      call SetupLevel5
      jp SelectLevel_end
  +:
    cp 3
    jr nz,+
      call SetupLevel10
      jp SelectLevel_end
  +:
    cp 4
    jr nz,+
      call SetupLevel11
      jp SelectLevel_end
  +:
    cp 5
    jr nz,+
      call SetupLevel12
      jp SelectLevel_end
  +:
    
  SelectLevel_end:
    ret

;-----------------------------------------------------------------------
endGame:
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
    ld sp, $dff0 ;clear stack
    jp main


;-----------------------------------------------------------------------
SetVScrollTo:
    ;input: target VScroll value in de (de must be positive)
    
    ;while YScrollTotal < de, incremente Vscroll
    ld hl,(YScrollTotal)
    ld a,h
    cp d
    jr z,SetVScrollTo_unit
    jr nc,+
      ;has to increment v scroll
      ld c,+1
      call VScrollManager
      jp SetVScrollTo ;recursive call
  +:
    ;has to decrement v scroll
    ld c,-1
    call VScrollManager
    jp SetVScrollTo ;recursive call
    
  SetVScrollTo_unit: ;less than 256 diff
    ld a,l
    cp e
    jr z,SetVScrollTo_end
    jr nc,+
      ;has to increment v scroll
      ld c,+1
      call VScrollManager
      jp SetVScrollTo ;recursive call
  +:
    ;has to decrement v scroll
    ld c,-1
    call VScrollManager
    jp SetVScrollTo ;recursive call
  
  SetVScrollTo_end:
    ret



