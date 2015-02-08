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
