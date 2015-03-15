ShowMenu:
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

    ;load default palette
    ld hl,Title_Palette_Start
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
    ; Clear tilemap data
    ;==============================================================
    ; 1. Set VRAM write address to name table index 0
    ; by outputting $4000 ORed with $3800+0
    ld a,$00
    out ($bf),a
    ld a,$38|$40
    out ($bf),a
    ; 2. Output tilemap data
    ;ld hl,Tilemap2Start
    ld bc,32*24*2; Counter for number of bytes to write
    -:
        xor a    ; Get data byte
        out ($be),a
        dec bc
        ld a,b
        or c
        jr nz,-

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
        
    ld hl,0
    ld (tiles_vram_used),hl
    ld bc,(FontData_End-FontData_Start)*4 ; *4 because 1bpp
    add hl,bc
    ld (tiles_vram_used),hl
    
    ;==============================================================
    ; Load title tiles
    ;==============================================================
    ; 1. Set VRAM write address to tile index 0
    ; by outputting $4000 ORed with (tiles_vram_used)
    ld a,l
    out ($bf),a
    ld a,h
    add a,$40
    out ($bf),a
    ; 2. Output tile data
    ld bc,Title_Tiles_End-Title_Tiles_Start  ; Counter for number of bytes to write
    ld hl,Title_Tiles_Start              ; Location of tile data
    
    -:
        ld a,(hl)        ; Get data byte
        out ($be),a
        inc hl           ; Add one to hl so it points to the next data byte
        dec bc
        ld a,b
        or c
        jp nz,-
    ld hl,(tiles_vram_used)
    ld bc,Title_Tiles_End-Title_Tiles_Start
    add hl,bc
    ld (tiles_vram_used),hl
    
    
    ;==============================================================
    ; Write tilemap data
    ;==============================================================
    ; 1. Set VRAM write address to name table index 0
    ; by outputting $4000 ORed with $3800+vram index
    ld a,$54
    out ($bf),a
    ld a,$38|$40|1
    out ($bf),a
    ; 2. Output tilemap data
    ld hl,Title_TileMap_Row1_Start
    ld bc,26  ; Counter for number of bytes to write
    -:
        ld a,(hl)    ; Get data byte
        out ($be),a
        inc hl       ; Point to next tile
        dec bc
        ld a,b
        or c
        jr nz,-  
    ld a,$94
    out ($bf),a
    ld a,$38|$40|1
    out ($bf),a
    ; 2. Output tilemap data
    ld hl,Title_TileMap_Row2_Start
    ld bc,26  ; Counter for number of bytes to write
    -:
        ld a,(hl)    ; Get data byte
        out ($be),a
        inc hl       ; Point to next tile
        dec bc
        ld a,b
        or c
        jr nz,-  
    ld a,$D4
    out ($bf),a
    ld a,$38|$40|1
    out ($bf),a
    ; 2. Output tilemap data
    ld hl,Title_TileMap_Row3_Start
    ld bc,26  ; Counter for number of bytes to write
    -:
        ld a,(hl)    ; Get data byte
        out ($be),a
        inc hl       ; Point to next tile
        dec bc
        ld a,b
        or c
        jr nz,-  


    ld c,3;col (tiles) in c
    ld l,22;line (tiles) in l
    ld de,Menu_Text_COPYRIGHT;text pointer in de
    ld a,1;have to write slowly
    call PrintText

    ld c,8;col (tiles) in c
    ld l,10;line (tiles) in l
    ld de,Menu_Text_1;text pointer in de
    ld a,1;have to write slowly
    call PrintText
    ld c,8;col (tiles) in c
    ld l,13;line (tiles) in l
    ld de,Menu_Text_2;text pointer in de
    ld a,1;have to write slowly
    call PrintText

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
    
    
    ;how the color loop works :
    ; changes colors 1-7 with Title_Colors_start
    ; index in colors increasing slowly
    ld de,0
  Title_loop:
      ;set palette of title
      ;==============================================================
      ; Update palette
      ;==============================================================
      ; 1. Set VRAM write address to CRAM (palette) address 1 (for palette index 1)
      ; by outputting $c000 ORed with $0001
      ld a,$01
      out ($bf),a
      ld a,$c0
      out ($bf),a
      ; 2. Output colour data
      ld hl,Title_Colors_start
      add hl,de
      ld b,7
      ld c,$be
      -:
        ld a,(hl)
        out ($be),a
        inc hl
        dec b
        jr nz,-
      
      inc de
      ld a,e
      cp 14
      jr nz,+
        ld e,0
      +:
      
      
      
      call WaitForVBlankSimple
      call Demo_PSGMOD_Play
      call WaitForVBlankSimple
      call Demo_PSGMOD_Play
      call WaitForVBlankSimple
      call Demo_PSGMOD_Play
      call WaitForVBlankSimple
      call Demo_PSGMOD_Play
      jp Title_loop
    ret

;TODO:
; - show menu
; - wait for input
; - setup game
;(add this file to project, call it)

Menu_Text_COPYRIGHT:
 .db "------ 2015  jmimu ------",0
 
Menu_Text_1:
 .db "DIFFICULTY LEVEL:",0
Menu_Text_2:
 .db "1 - 2 - 3 - 4 - 5",0

Title_Colors_start:
;.db $03,$03,$03,$03,$02,$02,$02,$02,$01,$01,$01,$01,$00,$00,$00,$00,$01,$01,$01,$01,$02,$02,$02,$02
;.db $03,$03,$03,$03,$02,$02,$02,$02,$01,$01,$01,$01,$00,$00,$00,$00,$01,$01,$01,$01,$02,$02,$02,$02

.db %00001001,%00001001,
.db %00011001,%00011001,
.db %00101001,%00101001,
.db %00111001,%00111001,
.db %00111101,%00111101,
.db %00101101,%00101101,
.db %00011101,%00011101,
.db %00001101,%00001101,
.db %00001001,%00001001,
.db %00011001,%00011001,
.db %00101001,%00101001,
.db %00111001,%00111001,
.db %00111101,%00111101,
.db %00101101,%00101101,
.db %00011101,%00011101,
.db %00001101,%00001101,

Title_Palette_Start:
.db %00000000,$03,$02,$02,$01,$01,$00,$00,$06,$0b,$01,$3d,$38,$0c,$08,$3c
.db $3f,$03,$02,$02,$01,$01,$00,$00,$3c,$0c,$0f,$08,$3a,$36,$01,$0a
Title_Palette_End:

Title_Tiles_Start:
.db $f8,$f8,$f8,$00,$f8,$f8,$f8,$00,$f8,$f8,$f8,$00,$f8,$f8,$f8,$00,$50,$f8,$f8,$00,$00,$f8,$f8,$00,$00,$f8,$f8,$00,$00,$f8,$f8,$00
.db $0f,$0f,$0f,$00,$7f,$7f,$7f,$00,$ff,$ff,$ff,$00,$ff,$ff,$ff,$00,$50,$f8,$f8,$00,$00,$f0,$f0,$00,$00,$e0,$e0,$00,$00,$00,$00,$00
.db $f8,$f8,$f8,$00,$fc,$fc,$fc,$00,$fe,$fe,$fe,$00,$ff,$ff,$ff,$00,$2a,$7f,$7f,$00,$00,$3f,$3f,$00,$00,$1f,$1f,$00,$00,$0f,$0f,$00
.db $f8,$00,$f8,$00,$d8,$00,$f8,$00,$20,$00,$f8,$00,$00,$00,$f8,$00,$00,$00,$f8,$00,$a8,$a8,$50,$00,$f8,$f8,$00,$00,$f8,$f8,$00,$00
.db $03,$00,$03,$00,$03,$00,$03,$00,$00,$00,$03,$00,$01,$00,$03,$00,$00,$00,$00,$00,$00,$00,$00,$00,$01,$01,$00,$00,$03,$03,$00,$00
.db $ff,$00,$ff,$00,$ff,$00,$ff,$00,$00,$00,$ff,$00,$02,$00,$83,$00,$00,$00,$01,$00,$4a,$4a,$b5,$00,$ff,$ff,$00,$00,$ff,$ff,$00,$00
.db $87,$00,$87,$00,$c7,$00,$c7,$00,$20,$00,$e7,$00,$00,$00,$e7,$00,$00,$00,$e7,$00,$e5,$e5,$02,$00,$47,$47,$a0,$00,$e7,$e7,$00,$00
.db $bf,$00,$bf,$00,$ff,$00,$ff,$00,$00,$00,$ff,$00,$84,$00,$ff,$00,$00,$00,$e7,$00,$81,$81,$42,$00,$c3,$c3,$00,$00,$83,$83,$00,$00
.db $80,$00,$80,$00,$c3,$00,$c3,$00,$40,$00,$c7,$00,$00,$00,$c7,$00,$00,$00,$c7,$00,$c5,$c5,$0a,$00,$4f,$4f,$80,$00,$cf,$cf,$00,$00
.db $fe,$00,$fe,$00,$ff,$00,$ff,$00,$08,$00,$ff,$00,$00,$00,$c7,$00,$00,$00,$83,$00,$00,$00,$81,$00,$00,$00,$00,$00,$00,$00,$00,$00
.db $f8,$00,$f8,$00,$e8,$00,$f8,$00,$28,$00,$f8,$00,$00,$00,$f8,$00,$00,$00,$f9,$00,$b9,$b9,$40,$00,$d9,$d9,$20,$00,$f9,$f9,$00,$00
.db $3f,$00,$3f,$00,$7f,$00,$7f,$00,$04,$00,$ff,$00,$40,$00,$f9,$00,$00,$00,$f0,$00,$a0,$a0,$40,$00,$e1,$e1,$00,$00,$ff,$ff,$00,$00
.db $83,$00,$83,$00,$e3,$00,$e3,$00,$20,$00,$f3,$00,$01,$00,$f3,$00,$00,$00,$73,$00,$b1,$b1,$42,$00,$e3,$e3,$10,$00,$f3,$f3,$00,$00
.db $c7,$00,$c7,$00,$ff,$00,$ff,$00,$05,$00,$ff,$00,$00,$00,$ff,$00,$00,$00,$f3,$00,$a0,$a0,$40,$00,$c0,$c0,$00,$00,$c0,$c0,$00,$00
.db $80,$00,$80,$00,$c0,$00,$c0,$00,$00,$00,$c0,$00,$00,$00,$c0,$00,$00,$00,$c0,$00,$00,$00,$01,$00,$03,$03,$00,$00,$07,$07,$00,$00
.db $0f,$00,$0f,$00,$1b,$00,$1f,$00,$08,$00,$3f,$00,$00,$00,$7e,$00,$00,$00,$fc,$00,$a8,$a8,$50,$00,$f0,$f0,$00,$00,$e0,$e0,$00,$00
.db $f8,$f8,$00,$00,$80,$f8,$00,$00,$20,$f8,$00,$00,$00,$f8,$00,$00,$25,$da,$00,$00,$ff,$00,$00,$00,$fb,$04,$00,$00,$ff,$00,$00,$00
.db $03,$03,$00,$00,$02,$03,$00,$00,$00,$03,$00,$00,$00,$03,$00,$00,$50,$ab,$00,$00,$5b,$a0,$00,$00,$f9,$00,$00,$00,$f8,$00,$00,$00
.db $ff,$ff,$00,$00,$42,$e3,$00,$00,$00,$c1,$00,$00,$00,$c1,$00,$00,$20,$c3,$00,$00,$df,$20,$00,$00,$ff,$00,$00,$00,$fe,$00,$00,$00
.db $67,$e7,$00,$00,$82,$e7,$00,$00,$40,$e7,$00,$00,$00,$e7,$00,$00,$80,$67,$00,$00,$e7,$00,$00,$00,$e7,$00,$00,$00,$e7,$00,$00,$00
.db $83,$83,$00,$00,$02,$83,$00,$00,$00,$83,$00,$00,$00,$83,$00,$00,$00,$83,$00,$00,$83,$00,$00,$00,$83,$00,$00,$00,$83,$00,$00,$00
.db $cf,$cf,$00,$00,$01,$cf,$00,$00,$84,$cf,$00,$00,$00,$cf,$00,$00,$00,$c7,$00,$00,$c3,$00,$00,$00,$c1,$00,$00,$00,$c0,$00,$00,$00
.db $00,$00,$00,$00,$01,$81,$00,$00,$00,$81,$00,$00,$00,$c3,$00,$00,$48,$a7,$00,$00,$6f,$90,$00,$00,$ff,$00,$00,$00,$fe,$00,$00,$00
.db $f9,$f9,$00,$00,$00,$f9,$00,$00,$50,$f9,$00,$00,$00,$f9,$00,$00,$80,$79,$00,$00,$78,$80,$00,$00,$f8,$00,$00,$00,$f8,$00,$00,$00
.db $ff,$ff,$00,$00,$84,$ff,$00,$00,$20,$e0,$00,$00,$00,$e0,$00,$00,$20,$d9,$00,$00,$fb,$04,$00,$00,$7f,$00,$00,$00,$3f,$00,$00,$00
.db $c3,$c3,$00,$00,$01,$03,$00,$00,$00,$03,$00,$00,$00,$f3,$00,$00,$a0,$53,$00,$00,$b2,$41,$00,$00,$e3,$00,$00,$00,$03,$00,$00,$00
.db $c0,$c0,$00,$00,$00,$c0,$00,$00,$00,$c0,$00,$00,$00,$c0,$00,$00,$40,$80,$00,$00,$c0,$00,$00,$00,$c0,$00,$00,$00,$c0,$00,$00,$00
.db $07,$0f,$00,$00,$12,$1f,$00,$00,$00,$3f,$00,$00,$00,$7f,$00,$00,$80,$7f,$00,$00,$ef,$10,$00,$00,$ff,$00,$00,$00,$ff,$00,$00,$00
.db $c0,$c0,$00,$00,$00,$80,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$54,$ab,$00,$00,$af,$50,$00,$00,$ff,$00,$00,$00,$ff,$00,$00,$00
Title_Tiles_End:

Title_TileMap_Row1_Start:
.dw  1+94,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  1+94,  0   ,  0   ,  0   ,  2+94,  3+94
Title_TileMap_Row2_Start:
.dw  4+94,  5+94,  6+94,  7+94,  8+94,  9+94, 10+94, 11+94, 12+94, 13+94, 14+94, 15+94, 16+94
Title_TileMap_Row3_Start:
.dw 17+94, 18+94, 19+94, 20+94, 21+94, 22+94, 23+94, 24+94, 25+94, 26+94, 27+94, 28+94, 29+94
