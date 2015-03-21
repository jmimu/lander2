Level12Ending:
    call WaitForVBlankLava
    
    call ResetSprites

    ld bc,-$4 ;//remove 4pix/256frame to y speed
    ld hl,(speedY)
    add hl,bc
    ld (speedY),hl
    ;draw fire sprite
    ld bc,(posX)
    ld a,b
    add a,$4;x+4
    ld h,a;x in h
    ld bc,(posY)
    ld a,b
    add a,$12;y+$12
    ld l,a;y in l
    ld d,fire_tile_number+1;number of the tile in VRAM in d
    call SpriteSet8x16
    
    ;change rocket status to bottom fire (1)
    ld a,1
    ld (rocket_status),a
    
    ;noise!
    ld c,%01100000;channel in c*%100000(max 3*%100000)
    call EnableChannel
    ld a,%00001000
    call PlayNoise

    ;mechanics
    ;increment Y-speed (gravity)
    ld hl,(speedY)
    inc hl
    inc hl
    ld (speedY),hl
    call UpdateVScroll

    ;update y pos
    ld bc,(posY)
    ld hl, (speedY)
    add hl,bc
    ld (posY),hl

    call DrawGameElements
    
    ;hide other sprites
    call SetLastSprite


    call UpdatePalette
    call UpdateScreen
    call PSGMOD_Play
    
    ;quit level when (posY)<5
    ld bc,(posY)
    ld a,b
    cp 5
    jp c,EndGame
    
  jp Level12Ending


EndGame:
  ;disable scrolling
  xor a
  out ($bf),a
  ld a,%10001000 ;x scroll
  out ($bf),a
  
  xor a
  out ($bf),a
  ld a,%10001001 ;y scroll
  out ($bf),a
  
  call InitializeDemo
  
  -:
    call WaitForVBlankWind
    
    call ResetSprites
    call DrawGameElements
    
    ;hide other sprites
    call SetLastSprite


    call UpdatePalette
    call UpdateScreen
    call PSGMOD_Play
  jp -
