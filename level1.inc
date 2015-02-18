.section "level1" force


SetupLevel1:
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
    ld hl,Palette1Start
    ld b,(Palette1End-Palette1Start)
    ld c,$be
    otir
    
    
    ld hl,(tiles_vram_used)
    ;==============================================================
    ; Load level tiles
    ;==============================================================
    ; 1. Set VRAM write address to tile index 0
    ; by outputting $4000 ORed with (tiles_vram_used)
    ld a,l
    out ($bf),a
    ld a,h
    add a,$40
    out ($bf),a
    ; 2. Output tile data
    ld bc,Bike_Tiles_Start-ForestTiles_Start  ; Counter for number of bytes to write
    ld hl,ForestTiles_Start              ; Location of tile data
    
    -:
        ld a,(hl)        ; Get data byte
        out ($be),a
        inc hl           ; Add one to hl so it points to the next data byte
        dec bc
        ld a,b
        or c
        jp nz,-
    ld hl,(tiles_vram_used)
    ld bc,Tiles_bg10_End-Tiles_bg10_Start
    add hl,bc
    ld (tiles_vram_used),hl

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
    ld bc,32*24*2; Counter for number of bytes to write
    -:
        ld a,(hl)    ; Get data byte
        out ($be),a
        inc hl       ; Point to next tile
        dec bc
        ld a,b
        or c
        jr nz,-
    
    
    ;init music
    ld hl,brahms1_start
    ld hl,demo_music_4_start
    call InitMusic1
    ld hl,brahms2_start
    call InitMusic2
    ld hl,brahms3_start
    call InitMusic3
    
    ;variables initialization
    ld hl,$0
    ld (speedX),hl
    ld hl,$0
    ld (speedY),hl
    ld hl,$8000
    ld (posX),hl
    ld hl,$8800
    ld (posY),hl
    xor a
    ld (rocket_status),a
    
    ;initialize scroll variables
    ld hl,Tilemap1Start
    ld (LevelMapStart),hl
    xor a
    ld (XScrollVDP),a
    ld (YScrollVDP),a
    ld hl,0
    ld (YScrollTotal),hl
    
    ;compute max y scroll value
    ld hl,0
    ld (YScrollTotalMax),hl
    
    ld hl,500
    ld (YLava),hl
    
    ;functers
    ld hl,WaitForVBlankWind
    ld (WaitForVBlankFunction),hl
    ld hl,DoWait
    ld (OnLandRightFunction),hl
    ld hl,DoWon
    ld (OnLandLeftFunction),hl
    ld hl,DoWon
    ld (OnOutFunction),hl
    
    ld hl,32
    ld (LevelMapWidth),hl
ret


WaitForVBlankWind:
    push af
    push hl
    
    ld hl,(Xscroll)
    ld bc,$50
    add hl,bc
    ld (Xscroll),hl
    
    ;no scroll at screen top
    xor a
    out ($bf),a
    ld a,%10001000
    out ($bf),a
    
    -:
      ;read scan line counter
      in a,($7E)
      ld c,a
      cp 36-1;if > 36, scroll quarter speed
      jr nz,+
      ld hl,(Xscroll)
      ld a,h
      out ($bf),a
      ld a,%10001000
      out ($bf),a
      
      +:
      ld a,c
      cp 68-1;if > 68, no scroll
      jr nz,+
      xor a
      out ($bf),a
      ld a,%10001000
      out ($bf),a

      +:
      
      ld a,(new_frame)
      cp 0
      jr z,-

      xor a
      ld (new_frame),a      
      
    pop hl
    pop af
  ret

;==============================================================
; Data
;==============================================================

Level1Start:

Palette1Start:
.db %00010100,$07,$00,$04,$06,$08,$0b,$10,$14,$15,$16,$1a,$29,$3f,$15,$0f
.db $3b,$3f,$00,$20,$38,$01,$02,$03,$15,$2a,$1f,$2f,$3a,$36,$01,$0a
Palette1End:

Tilemap1Start:
_TilemapStart:
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010101101 %0000000010101110 %0000000010101111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010110000 %0000000010110001 %0000000010110010 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010110011 %0000000010110100 %0000000010110101 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010100011
.dw %0000000010100100 %0000000010100111 %0000000010101000 %0000000010101001 %0000000010101010 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010100011 %0000000010101001 %0000000010101010
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010100101 %0000000010100110
.dw %0000000010100111 %0000000010101011 %0000000010101100 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010100101 %0000000010101011 %0000000010101100 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010001001 %0000000010001010 %0000000010001001 %0000000010001010
.dw %0000000010001001 %0000000010001010 %0000000010001001 %0000000010001010 %0000000010001001 %0000000010001010 %0000000010000111 %0000000010000111
.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010001001 %0000000010001010
.dw %0000000010001001 %0000000010001010 %0000000010001001 %0000000010001010 %0000000010001011 %0000000010001100 %0000000010001011 %0000000010001100
.dw %0000000010001011 %0000000010001100 %0000000010001011 %0000000010001100 %0000000010001011 %0000000010001100 %0000000010001001 %0000000010001010
.dw %0000000010001001 %0000000010001010 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111 %0000000010000111

.dw %0000000010000111 %0000000010000111 %0000000010001001 %0000000010001010 %0000000010001001 %0000000010001010 %0000000010001011 %0000000010001100
.dw %0000000010001011 %0000000010001100 %0000000010001011 %0000000010001100 %0000000010001101 %0000000010001110 %0000000010001101 %0000000010001110
.dw %0000000010001101 %0000000010001110 %0000000010001101 %0000000010001110 %0000000010001101 %0000000010001110 %0000000010001011 %0000000010001100
.dw %0000000010001011 %0000000010001100 %0000000010001001 %0000000010001010 %0000000010001001 %0000000010001010 %0000000010000111 %0000000010000111

.dw %0000000010001001 %0000000010001010 %0000000010001011 %0000000010001100 %0000000010001011 %0000000010001100 %0000000010001101 %0000000010001110
.dw %0000000010001101 %0000000010001110 %0000000010001101 %0000000010001110 %0000000010001111 %0000000010010000 %0000000010001111 %0000000010010000
.dw %0000000010001111 %0000000010010000 %0000000010001111 %0000000010010000 %0000000010001111 %0000000010010000 %0000000010001101 %0000000010001110
.dw %0000000010001101 %0000000010001110 %0000000010001011 %0000000010001100 %0000000010001011 %0000000010001100 %0000000010001001 %0000000010001010

.dw %0000000010001011 %0000000010001100 %0000000010001101 %0000000010001110 %0000000010001101 %0000000010001110 %0000000010001111 %0000000010010000
.dw %0000000010001111 %0000000010010000 %0000000010001111 %0000000010010000 %0000000010010101 %0000000010011110 %0000000010010101 %0000000010011110
.dw %0000000010010101 %0000000010011110 %0000000010010101 %0000000010011110 %0000000010010101 %0000000010011110 %0000000010001111 %0000000010010000
.dw %0000000010001111 %0000000010010000 %0000000010001101 %0000000010001110 %0000000010001101 %0000000010001110 %0000000010001011 %0000000010001100

.dw %0000000010001101 %0000000010001110 %0000000010001111 %0000000010010000 %0000000010001111 %0000000010010000 %0000000010010101 %0000000010011110
.dw %0000000010010101 %0000000010011110 %0000000010010101 %0000000010011110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110
.dw %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010010101 %0000000010011110
.dw %0000000010010101 %0000000010011110 %0000000010001111 %0000000010010000 %0000000010001111 %0000000010010000 %0000000010001101 %0000000010001110

.dw %0000000010001111 %0000000010010000 %0000000010010101 %0000000010011110 %0000000010010101 %0000000010011110 %0000000010111110 %0000000010111110
.dw %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111000 %0000000010111101 %0000000010111011 %0000000010111101
.dw %0000000010111101 %0000000010111101 %0000000010111011 %0000000010111101 %0000000010110111 %0000000010111110 %0000000010111110 %0000000010111100
.dw %0000000010111110 %0000000010111110 %0000000010010101 %0000000010011110 %0000000010010101 %0000000010011110 %0000000010001111 %0000000010010000

.dw %0000000010010101 %0000000010011110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110
.dw %0000000010111110 %0000000010111110 %0000000010111000 %0000000010111101 %0000000010111101 %0000000010100001 %0000000010100001 %0000000010100001
.dw %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010111011 %0000000010111101 %0000000010111101 %0000000010110111
.dw %0000000010111110 %0000000010111100 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010010101 %0000000010011110

.dw %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111100 %0000000010111110 %0000000010111100 %0000000010111110 %0000000010111110
.dw %0000000010111000 %0000000010111011 %0000000010111101 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001
.dw %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010111011 %0000000010111101
.dw %0000000010110111 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110

.dw %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111000
.dw %0000000010111101 %0000000010111101 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100010 %0000000010100010
.dw %0000000010100010 %0000000010100010 %0000000010100010 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010111011
.dw %0000000010111101 %0000000010111110 %0000000010111100 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110 %0000000010111110

.dw %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001
.dw %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001
.dw %0000000010100010 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001 %0000000010100001
.dw %0000000010111101 %0000000010110111 %0000000010111110 %0000000010111110 %0000000010111100 %0000000010111110 %0000000010111110 %0000000010111110

.dw %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001
.dw %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %1110000010100001 %1110000010100001
.dw %1110001010100010 %1110000010100001 %1110000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001
.dw %0010000010111101 %0010000010111101 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110

.dw %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111010
.dw %0010000010111011 %0010000010111101 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100010 %0010000010100010
.dw %0010000010100010 %0010000010100010 %0010000010100010 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010111101
.dw %0010000010111101 %0010000010111001 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110

.dw %0010000010111110 %0010000010111100 %0010000010111110 %0010000010111110 %0010000010111100 %0010000010111110 %0010000010111110 %0010000010111110
.dw %0010000010111110 %0010000010111010 %0010000010111011 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001
.dw %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010111101 %0010000010111101
.dw %0010000010111001 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111100 %0010000010111110

.dw %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111100 %0010000010111100 %0010000010111110
.dw %0010000010111110 %0010000010111110 %0010000010111010 %0010000010111101 %0010000010111101 %0010000010100001 %0010000010100001 %0010000010100001
.dw %0010000010100001 %0010000010100001 %0010000010100001 %0010000010100001 %0010000010111101 %0010000010111011 %0010000010111101 %0010000010111001
.dw %0010000010111110 %0010000010111100 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110 %0010000010111110


Tilemap1End:
Level1End:



.ends ;--------------- end level1 section
