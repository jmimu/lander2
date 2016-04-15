.ramsection "Sprites variables" slot 1
  SATbuf dsb 256    ;Sprite Attribute Table buffer
  number_of_sprites     db ; number of sprites to draw this frame
.ends

.section "sprites functions" free


; --------------------------------------------------------------
; PREPARE VRAM.
; Set up vdp to recieve data at vram address in HL.

VRAMsetAddress:
  push af
  ld a,l
  out ($bf),a
  ld a,h
  or $40
  out ($bf),a
  pop af
ret

; --------------------------------------------------------------
; WRITE TO VRAM
; Write BC amount of bytes from data source pointed to by HL.
; Tip: Use VRAMsetAddress before calling.

VRAMwrite:
  ld a,(hl)
  out ($be),a
  inc hl
  dec bc
  ld a,c
  or b
  jp nz,VRAMwrite
ret

; --------------------------------------------------------------
; LOAD SPRITE ATTRIBUTE TABLE
; Load data into sprite attribute table (SAT) from the buffer.
;TODO: use VRAM_write to write 256 bytes?

UpdateSAT:
  push hl
  push bc
  ld hl,$3f00         ; point to start of SAT in vram.
  call VRAMsetAddress   ; prepare vram to recieve data.
  ld b,$40            ; amount of bytes to output.
  ld c,$be            ; destination is vdp data port.
  ld hl,SATbuf        ; source is start of sat buffer.
  otir                ; output buffer to vdp.

  ld hl,$3f80         ; point to start of SAT in vram.
  call VRAMsetAddress   ; prepare vram to recieve data.
  ld b,$80            ; amount of bytes to output.
  ld c,$be            ; destination is vdp data port.
  ld hl,SATbuf+$80        ; source is start of sat buffer.
  otir                ; output buffer to vdp.

  pop bc
  pop hl
ret

;--------------------------------------------------------------------
; input: offset in SATbuf in a
; output: HL = RAM address of SATbuf byte
GetSATbufAddr:
  push bc
  ld b,0
  ld c,a
  ld hl,SATbuf
  add hl,bc
  pop bc
ret

;--------------------------------------------------------------------
ResetSprites:
  xor a
  ld (number_of_sprites),a
ret


;--------------------------------------------------------------------
SetLastSprite:
  push bc
  push hl
  
  ld a,(number_of_sprites)
  call GetSATbufAddr
  
  ld a,$D0
  ld (hl),a ;set $D0 to the y of sprite e+1 = e is the last sprite to print

  pop hl
  pop bc
ret



;--------------------------------------------------------------------
HideSprites:
  call ResetSprites
  call SetLastSprite
ret

;--------------------------------------------------------------------
SpriteSet8x8:
  ;x in h
  ;y in l
  ;n in d
  push bc
  push hl
  push de
  
  ld b,h ;x
  ld c,l ;y
  ld a,(number_of_sprites)
  ld e,a;sprite index in e
  inc a
  ld (number_of_sprites),a
  
  ;y
  ld a,e
  call GetSATbufAddr
  ld (hl),c
  ;x n
  ld a, e
  add a, a
  or $80
  call GetSATbufAddr
  ld (hl),b
  inc hl
  ld (hl),d
  
  pop de
  pop hl
  pop bc
ret

;--------------------------------------------------------------------
SpriteSet16x8:
  ;x in h
  ;y in l
  ;n in d
  push hl
  push de
  
  call SpriteSet8x8

  inc d
  ld a,h
  add a,8
  ld h,a
  call SpriteSet8x8

  pop de
  pop hl
ret

;
;--------------------------------------------------------------------
SpriteSet16x16:
  ;x in h
  ;y in l
  ;n in d
  push hl
  push de

  call SpriteSet8x8

  inc d
  ld a,h
  add a,8
  ld h,a
  call SpriteSet8x8

  inc d
  ld a,h
  add a,-8
  ld h,a
  ld a,l
  add a,8
  ld l,a
  call SpriteSet8x8
  
  inc d
  ld a,h
  add a,8
  ld h,a
  call SpriteSet8x8
  
  pop de
  pop hl
ret

;--------------------------------------------------------------------
SpriteSet8x16:
  ;x in h
  ;y in l
  ;n in d
  push hl
  push de

  call SpriteSet8x8

  inc d
  ld a,l
  add a,8
  ld l,a
  call SpriteSet8x8

  pop de
  pop hl
ret





.ends
