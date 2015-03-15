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
    ret

;TODO:
; - show menu
; - wait for input
; - setup game
;(add this file to project, call it)



Title_Palette_Start:
.db $3f,$00,$00,$01,$01,$02,$02,$03,$06,$0b,$01,$3d,$38,$0c,$08,$3c
.db $3f,$00,$00,$01,$01,$02,$02,$03,$3c,$0c,$0f,$08,$3a,$36,$01,$0a
Title_Palette_End:

Title_Tiles_Start:
.db $f8,$00,$00,$00,$f8,$00,$00,$00,$f8,$00,$00,$00,$f8,$00,$00,$00,$50,$a8,$00,$00,$00,$f8,$00,$00,$00,$f8,$00,$00,$00,$f8,$00,$00
.db $0f,$00,$00,$00,$7f,$00,$00,$00,$ff,$00,$00,$00,$ff,$00,$00,$00,$50,$a8,$00,$00,$00,$f0,$00,$00,$00,$e0,$00,$00,$00,$00,$00,$00
.db $f8,$00,$00,$00,$fc,$00,$00,$00,$fe,$00,$00,$00,$ff,$00,$00,$00,$2a,$55,$00,$00,$00,$3f,$00,$00,$00,$1f,$00,$00,$00,$0f,$00,$00
.db $f8,$f8,$00,$00,$d8,$d8,$20,$00,$20,$20,$d8,$00,$00,$00,$f8,$00,$00,$00,$f8,$00,$a8,$00,$f8,$00,$f8,$00,$f8,$00,$f8,$00,$f8,$00
.db $03,$03,$00,$00,$03,$03,$00,$00,$00,$00,$03,$00,$01,$01,$02,$00,$00,$00,$00,$00,$00,$00,$00,$00,$01,$00,$01,$00,$03,$00,$03,$00
.db $ff,$ff,$00,$00,$ff,$ff,$00,$00,$00,$00,$ff,$00,$02,$02,$81,$00,$00,$00,$01,$00,$4a,$00,$ff,$00,$ff,$00,$ff,$00,$ff,$00,$ff,$00
.db $87,$87,$00,$00,$c7,$c7,$00,$00,$20,$20,$c7,$00,$00,$00,$e7,$00,$00,$00,$e7,$00,$e5,$00,$e7,$00,$47,$00,$e7,$00,$e7,$00,$e7,$00
.db $bf,$bf,$00,$00,$ff,$ff,$00,$00,$00,$00,$ff,$00,$84,$84,$7b,$00,$00,$00,$e7,$00,$81,$00,$c3,$00,$c3,$00,$c3,$00,$83,$00,$83,$00
.db $80,$80,$00,$00,$c3,$c3,$00,$00,$40,$40,$87,$00,$00,$00,$c7,$00,$00,$00,$c7,$00,$c5,$00,$cf,$00,$4f,$00,$cf,$00,$cf,$00,$cf,$00
.db $fe,$fe,$00,$00,$ff,$ff,$00,$00,$08,$08,$f7,$00,$00,$00,$c7,$00,$00,$00,$83,$00,$00,$00,$81,$00,$00,$00,$00,$00,$00,$00,$00,$00
.db $f8,$f8,$00,$00,$e8,$e8,$10,$00,$28,$28,$d0,$00,$00,$00,$f8,$00,$00,$00,$f9,$00,$b9,$00,$f9,$00,$d9,$00,$f9,$00,$f9,$00,$f9,$00
.db $3f,$3f,$00,$00,$7f,$7f,$00,$00,$04,$04,$fb,$00,$40,$40,$b9,$00,$00,$00,$f0,$00,$a0,$00,$e0,$00,$e1,$00,$e1,$00,$ff,$00,$ff,$00
.db $83,$83,$00,$00,$e3,$e3,$00,$00,$20,$20,$d3,$00,$01,$01,$f2,$00,$00,$00,$73,$00,$b1,$00,$f3,$00,$e3,$00,$f3,$00,$f3,$00,$f3,$00
.db $c7,$c7,$00,$00,$ff,$ff,$00,$00,$05,$05,$fa,$00,$00,$00,$ff,$00,$00,$00,$f3,$00,$a0,$00,$e0,$00,$c0,$00,$c0,$00,$c0,$00,$c0,$00
.db $80,$80,$00,$00,$c0,$c0,$00,$00,$00,$00,$c0,$00,$00,$00,$c0,$00,$00,$00,$c0,$00,$00,$00,$01,$00,$03,$00,$03,$00,$07,$00,$07,$00
.db $0f,$0f,$00,$00,$1b,$1b,$04,$00,$08,$08,$37,$00,$00,$00,$7e,$00,$00,$00,$fc,$00,$a8,$00,$f8,$00,$f0,$00,$f0,$00,$e0,$00,$e0,$00
.db $f8,$00,$f8,$00,$80,$78,$f8,$00,$20,$d8,$f8,$00,$00,$f8,$f8,$00,$25,$ff,$ff,$00,$ff,$ff,$ff,$00,$fb,$ff,$ff,$00,$ff,$ff,$ff,$00
.db $03,$00,$03,$00,$02,$01,$03,$00,$00,$03,$03,$00,$00,$03,$03,$00,$50,$fb,$fb,$00,$5b,$fb,$fb,$00,$f9,$f9,$f9,$00,$f8,$f8,$f8,$00
.db $ff,$00,$ff,$00,$42,$a1,$e3,$00,$00,$c1,$c1,$00,$00,$c1,$c1,$00,$20,$e3,$e3,$00,$df,$ff,$ff,$00,$ff,$ff,$ff,$00,$fe,$fe,$fe,$00
.db $67,$80,$e7,$00,$82,$65,$e7,$00,$40,$a7,$e7,$00,$00,$e7,$e7,$00,$80,$e7,$e7,$00,$e7,$e7,$e7,$00,$e7,$e7,$e7,$00,$e7,$e7,$e7,$00
.db $83,$00,$83,$00,$02,$81,$83,$00,$00,$83,$83,$00,$00,$83,$83,$00,$00,$83,$83,$00,$83,$83,$83,$00,$83,$83,$83,$00,$83,$83,$83,$00
.db $cf,$00,$cf,$00,$01,$ce,$cf,$00,$84,$4b,$cf,$00,$00,$cf,$cf,$00,$00,$c7,$c7,$00,$c3,$c3,$c3,$00,$c1,$c1,$c1,$00,$c0,$c0,$c0,$00
.db $00,$00,$00,$00,$01,$80,$81,$00,$00,$81,$81,$00,$00,$c3,$c3,$00,$48,$ef,$ef,$00,$6f,$ff,$ff,$00,$ff,$ff,$ff,$00,$fe,$fe,$fe,$00
.db $f9,$00,$f9,$00,$00,$f9,$f9,$00,$50,$a9,$f9,$00,$00,$f9,$f9,$00,$80,$f9,$f9,$00,$78,$f8,$f8,$00,$f8,$f8,$f8,$00,$f8,$f8,$f8,$00
.db $ff,$00,$ff,$00,$84,$7b,$ff,$00,$20,$c0,$e0,$00,$00,$e0,$e0,$00,$20,$f9,$f9,$00,$fb,$ff,$ff,$00,$7f,$7f,$7f,$00,$3f,$3f,$3f,$00
.db $c3,$00,$c3,$00,$01,$02,$03,$00,$00,$03,$03,$00,$00,$f3,$f3,$00,$a0,$f3,$f3,$00,$b2,$f3,$f3,$00,$e3,$e3,$e3,$00,$03,$03,$03,$00
.db $c0,$00,$c0,$00,$00,$c0,$c0,$00,$00,$c0,$c0,$00,$00,$c0,$c0,$00,$40,$c0,$c0,$00,$c0,$c0,$c0,$00,$c0,$c0,$c0,$00,$c0,$c0,$c0,$00
.db $07,$08,$0f,$00,$12,$0d,$1f,$00,$00,$3f,$3f,$00,$00,$7f,$7f,$00,$80,$ff,$ff,$00,$ef,$ff,$ff,$00,$ff,$ff,$ff,$00,$ff,$ff,$ff,$00
.db $c0,$00,$c0,$00,$00,$80,$80,$00,$00,$00,$00,$00,$00,$00,$00,$00,$54,$ff,$ff,$00,$af,$ff,$ff,$00,$ff,$ff,$ff,$00,$ff,$ff,$ff,$00
Title_Tiles_End:

Title_TileMap_Row1_Start:
.dw  1+95,  0   ,  0   ,  0   ,  0   ,  0   ,  0   ,  1+95,  0   ,  0   ,  0   ,  2+95,  3
Title_TileMap_Row2_Start:
.dw  4+95,  5+95,  6+95,  7+95,  8+95,  9+95, 10+95, 11+95, 12+95, 13+95, 14+95, 15+95, 16
Title_TileMap_Row3_Start:
.dw 17+95, 18+95, 19+95, 20+95, 21+95, 22+95, 23+95, 24+95, 25+95, 26+95, 27+95, 28+95, 29
