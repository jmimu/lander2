Multby32:
    ;mult l by 32, result in hl
    push af
        ld h,$00
        ld a,5 ; 32=2^5
     -: sla h ;//left shift for MSB
        sla l ;//left shift for LSB
        jr nc,+ ;if carry on sla l
        inc h
     +:
        dec a
        jr nz,-
    pop af
    ret


PrintInt: ;print an int in octal...
    ;col (tiles) in c
    ;line (tiles) in l
    ;value (8bit) in e
    ; the constant "digits_tile_number" must represent the number of the "0" tile in VRAM
    
    push af
    push bc
    push de
    push hl
    
    ;compute the tile index in tilemap
    ;sms screen is 32*28 tiles large
    call Multby32 ;multipy l by 32, result in hl
    ld b,0
    add hl,bc
    ld bc,2
    add hl,bc ;begin to write the least siginificant digit
    add hl,hl ;2 bytes by tile
    ;hl is the number of the byte in tilemap
    
    di
    
    ;write digits
    ld c,3
  -:
        ;==============================================================
        ; Write text to name table
        ;==============================================================
        ; 1. Set VRAM write address to name table index 0
        ; by outputting $4000 ORed with $3800+hl*2
        ld a,$00
        add a,l ;number of byte: hl
        out ($bf),a
        ld a,$38|$40
        add a,h ;number of byte: hl
        out ($bf),a
        
        ; 2. Output tilemap data   
        ld a,e
        and %00000111 ;get only the last significant digit
        ld b,a
        ld a,digits_tile_number ;tile to write : digits_tile_number+value
        add a,b
        out ($be),a
        
        ;ld a,e
        srl e ;divide by 8
        srl e
        srl e
        ;ld e,a
        
        dec hl ; wel'll draw next tile (go left)
        dec hl ; wel'll draw next tile (go left)
        dec c
        jr nz,-
    
    ei
    
    pop hl
    pop de
    pop bc
    pop af
        
    ret

PrintText:
    ;col (tiles) in c
    ;line (tiles) in l
    ;text pointer in de
    ;text length in b
    ;the constant "diff_tile_ascii"  must represent difference between index in tiles and in ascii
    
    ;push bc
    ;  ld b,e
    ;  call PrintInt
    ;pop bc
    
    push bc
    push de
    push hl
    push af
    push bc
    
    
    ;compute the tile index in tilemap
    ;sms screen is 32*28 tiles large
    call Multby32 ;multipy l by 32, result in hl
    ld b,0
    add hl,bc
    ;hl is the number of the tile
    
    add hl,hl ;2 bytes by tile
    ;hl is the number of the byte in tilemap
    
    pop bc
    
    
    ;write chars
    -:
        ;==============================================================
      ; Write text to name table
      ;==============================================================
      ; 1. Set VRAM write address to name table index 0
      ; by outputting $4000 ORed with $3800+hl*2
      ld a,$00
      add a,l ;number of byte: hl
      out ($bf),a
      ld a,$38|$40
      add a,h ;number of byte: hl
      out ($bf),a
      
      ; 2. Output tilemap data   
      ld a,(de);the char
      sub diff_tile_ascii
      out ($be),a
      xor a ;one zero
      out ($be),a
      inc de;move pointer in data
      inc hl;move pointer on screen
      inc hl;move pointer on screen
      
      dec b
      jr nz,-
    
    
    pop af
    pop hl
    pop de
    pop bc
    
        
    ret
    
    
PrintTextStatic: ;for static screens (turn off screen to avoid missing letters)
    ;col (tiles) in c
    ;line (tiles) in l
    ;text pointer in de
    ;text length in b
    ;the constant "diff_tile_ascii"  must represent difference between index in tiles and in ascii
    
    ;push bc
    ;  ld b,e
    ;  call PrintInt
    ;pop bc
    
    push bc
    push de
    push hl
    push af
    push bc
    
    
    ;compute the tile index in tilemap
    ;sms screen is 32*28 tiles large
    call Multby32 ;multipy l by 32, result in hl
    ld b,0
    add hl,bc
    ;hl is the number of the tile
    
    add hl,hl ;2 bytes by tile
    ;hl is the number of the byte in tilemap
    
    pop bc
    
    di
    ; Turn screen off
    ld a,%10100000
;          |||| |`- Zoomed sprites -> 16x16 pixels
;          |||| `-- Doubled sprites -> 2 tiles per sprite, 8x16
;          |||`---- 30 row/240 line mode
;          ||`----- 28 row/224 line mode
;          |`------ VBlank interrupts
;          `------- Enable display
    out ($bf),a
    ld a,$81
    out ($bf),a
    
    ;write chars
    -:
        ;==============================================================
      ; Write text to name table
      ;==============================================================
      ; 1. Set VRAM write address to name table index 0
      ; by outputting $4000 ORed with $3800+hl*2
      ld a,$00
      add a,l ;number of byte: hl
      out ($bf),a
      ld a,$38|$40
      add a,h ;number of byte: hl
      out ($bf),a
      
      ; 2. Output tilemap data   
      ld a,(de);the char
      sub diff_tile_ascii
      out ($be),a
      xor a ;one zero
      out ($be),a
      inc de;move pointer in data
      inc hl;move pointer on screen
      inc hl;move pointer on screen
      
      dec b
      jr nz,-
    

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
    ei
    
    
    pop af
    pop hl
    pop de
    pop bc
    
        
    ret
