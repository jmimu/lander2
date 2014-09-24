#!/usr/bin/python3
import os
import sys
import json

if (len(sys.argv)<2):
  print("Synthax: python3 tiled2asm.py json_file")
  exit()

filename=sys.argv[1]
data = json.load(open(filename))

uncompressed_map=data["layers"][0]["data"]
height=data["layers"][0]["height"]
width=data["layers"][0]["width"]


tilesets=data["tilesets"]
back_tileset=None

for tileset in tilesets:
  if tileset["name"]=="back":
    back_tileset=tileset
  if tileset["name"]=="collisions":
    collisions_tileset=tileset

if back_tileset:
  back_tileset_w=int(back_tileset["imagewidth"]/back_tileset["tilewidth"])
  #height is divided by 2 because lower half is first half flipped
  back_tileset_h=int(back_tileset["imageheight"]/back_tileset["tileheight"]/2)
  back_tileset_firstgid=back_tileset["firstgid"]
  back_tileset_nbr_tiles=back_tileset_w*back_tileset_h
  print("back_tileset_w: ",back_tileset_w)
  print("back_tileset_h: ",back_tileset_h)
else:
  print("Error, no tileset \"back\"!")
  exit()


if collisions_tileset:
  collisions_tileset_w=int(collisions_tileset["imagewidth"]/collisions_tileset["tilewidth"])
  #height is divided by 2 because lower half is first half flipped
  collisions_tileset_h=int(collisions_tileset["imageheight"]/collisions_tileset["tileheight"]/2)
  collisions_tileset_firstgid=collisions_tileset["firstgid"]
  collisions_tileset_nbr_tiles=collisions_tileset_w*collisions_tileset_h
  print("collisions_tileset_w: ",collisions_tileset_w)
  print("collisions_tileset_h: ",collisions_tileset_h)
else:
  print("Error, no tileset \"collisions\"!")
  exit()






k=0
print("_TilemapStart:")
for i in range(height):
  str=".dw"
  for j in range(width):
    str+=' $%04x'%(uncompressed_map[k]-1)
    if j==15:
      str+="\n.dw"
    k+=1
  print(str)
print("_TilemapEnd:")

