;--------------------------------------------------------------------
SetLastSprite:
  ;sprite index in e
  ld a, e
  inc a
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
  ;sprite index in e

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
  ;sprite index in e

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
  ;sprite index in e

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

;-------------------------------------------------------------------
DrawRocket
  ;x in h
  ;y in l
  ;sprite index in e
    ;top part
    ld d,rocket_tile_number;number of the tile in VRAM in d
    ;test if big_rocket
    ld a,(big_rocket)
    cp 0
    jr nz,+
    inc d;if small rocket start 2 tiles later
    inc d
  +: 
    call SpriteSet16x8
    ;body part
    ld a,l
    add a,8
    ld l,a;y+8
    ld d,rocket_tile_number+4
    inc e
    inc e
    call SpriteSet16x16

  ret

;--------------------------------------------------------------------
SpriteSet16x24:
  ;x in h
  ;y in l
  ;n in d
  ;sprite index in e

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

