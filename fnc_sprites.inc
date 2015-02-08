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
