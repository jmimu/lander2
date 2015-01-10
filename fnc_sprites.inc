.ramsection "Sprites variables" slot 1
  number_of_sprites     db ; number of sprites to draw this frame
.ends

.section "sprites functions" free

;--------------------------------------------------------------------
ResetSprites:
  xor a
  ld (number_of_sprites),a
ret


;--------------------------------------------------------------------
SetLastSprite:
  ld a,(number_of_sprites)
  ld e,a;sprite index in e
  ld a, e
  ;inc a
  out ($bf), a
  ld a, $7f
  out ($bf), a
  
  ld a, $D0 ;set $D0 to the y of sprite e+1 = e is the last sprite to print
  out ($be), a
ret

;--------------------------------------------------------------------
HideSprites:
  ;first sprite to hide index in a
  ;number of sprites to hide in c
  out ($bf), a
  ld a, $7f
  out ($bf), a
-:
  ld a, $D0 ;set $D0 to the y of sprite e+1 = e is the last sprite to print
  out ($be), a
  dec c
  jr nz,-
ret

;--------------------------------------------------------------------
SpriteSet8x8:
  ;x in h
  ;y in l
  ;n in d
  ld a,(number_of_sprites)
  ld e,a;sprite index in e
  inc a
  ld (number_of_sprites),a


  ;vdp set addr
  ld a, e
  out ($bf), a
  ld a, $7f
  out ($bf), a
  ;y
  ld a, l
  out ($be), a
  ;vdp set addr
  ld a, e
  add a, a
  or $80
  out ($bf), a
  ld a, $7f
  out ($bf), a
  ;x n
  ld a, h
  out ($be), a
  ld a, d
  out ($be), a
ret

;--------------------------------------------------------------------
SpriteSet16x8:
  ;x in h
  ;y in l
  ;n in d
  ld a,(number_of_sprites)
  ld e,a;sprite index in e
  inc a
  inc a
  ld (number_of_sprites),a


  ;vdp set addr
  ld a, e
  out ($bf), a
  ld a, $7f
  out ($bf), a

  ;y+0 y+0
  ld a, l
  out ($be), a
  out ($be), a

  ;vdp set addr
  ld a, e
  add a, a
  or $80
  out ($bf), a
  ld a, $7f
  out ($bf), a

  ;x+0 n+0 x+8 n+1
  ld a, h
  out ($be), a
  ld a, d
  out ($be), a

  ld a, h
  add a, $08
  out ($be), a
  inc d
  ld a, d
  out ($be), a

ret

;
;--------------------------------------------------------------------
SpriteSet16x16:
  ;x in h
  ;y in l
  ;n in d
  ld a,(number_of_sprites)
  ld a,(number_of_sprites)
  ld e,a;sprite index in e
  add a,4
  ld (number_of_sprites),a


  ;vdp set addr
  ld a, e
  out ($bf), a
  ld a, $7f
  out ($bf), a

  ;y+0 y+0
  ;y+8 y+8
  ld a, l
  out ($be), a
  out ($be), a
  add a, $08
  out ($be), a
  out ($be), a

  ;vdp set addr
  ld a, e
  add a, a
  or $80
  out ($bf), a
  ld a, $7f
  out ($bf), a

  ;x+0 n+0 x+8 n+1
  ;x+0 n+2 x+8 n+3
  ld a, h
  out ($be), a
  ld a, d
  out ($be), a

  ld a, h
  add a, $08
  out ($be), a
  inc d
  ld a, d
  out ($be), a

  ld a, h
  out ($be), a
  inc d
  ld a, d
  out ($be), a

  ld a, h
  add a, $08
  out ($be), a
  inc d
  ld a, d
  out ($be), a
ret

;--------------------------------------------------------------------
SpriteSet8x16:
  ;x in h
  ;y in l
  ;n in d
  ld a,(number_of_sprites)
  ld e,a;sprite index in e
  inc a
  inc a
  ld (number_of_sprites),a


  ;vdp set addr
  ld a, e
  out ($bf), a
  ld a, $7f
  out ($bf), a

  ;y+0
  ;y+8
  ld a, l
  out ($be), a
  add a, $08
  out ($be), a

  ;vdp set addr
  ld a, e
  add a, a
  or $80
  out ($bf), a
  ld a, $7f
  out ($bf), a

  ;x+0 n+0
  ;x+0 n+1
  ld a, h
  out ($be), a
  ld a, d
  out ($be), a

  ld a, h
  out ($be), a
  inc d
  ld a, d
  out ($be), a
ret

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

;--------------------------------------------------------------------
SpriteSet16x24:
  ;x in h
  ;y in l
  ;n in d
  ld a,(number_of_sprites)
  ld e,a;sprite index in e
  add a,6
  ld (number_of_sprites),a

  ;vdp set addr
  ld a, e
  out ($bf), a
  ld a, $7f
  out ($bf), a

  ;y+0 y+0
  ;y+8 y+8
  ;y+16 y+16
  ld a, l
  out ($be), a
  out ($be), a
  add a, $08
  out ($be), a
  out ($be), a
  add a, $08
  out ($be), a
  out ($be), a
  
  ;vdp set addr
  ld a, e
  add a, a
  or $80
  out ($bf), a
  ld a, $7f
  out ($bf), a

  ;x+0 n+0 x+8 n+1
  ;x+0 n+2 x+8 n+3
  ;x+0 n+4 x+8 n+5
  ld a, h
  out ($be), a
  ld a, d
  out ($be), a

  ld a, h
  add a, $08
  out ($be), a
  inc d
  ld a, d
  out ($be), a

  ld a, h
  out ($be), a
  inc d
  ld a, d
  out ($be), a

  ld a, h
  add a, $08
  out ($be), a
  inc d
  ld a, d
  out ($be), a

  ld a, h
  out ($be), a
  inc d
  ld a, d
  out ($be), a

  ld a, h
  add a, $08
  out ($be), a
  inc d
  ld a, d
  out ($be), a
 
ret

.ends
