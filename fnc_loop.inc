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

    ;check if end of game
    ld a,(current_level)
    dec a
    cp number_of_levels
    jr nz,+
    call endGame
   +:
   
    ;check if fuel reset
    ld a,(refuel)
    cp 0
    jr z,+
      ld hl,$FF0F
      ld (rocket_fuel),hl
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
    
    ld a,1
    ld (new_frame),a

    ei;enable interruption (for vblank)    

    ;set game status to level start
    xor a
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
