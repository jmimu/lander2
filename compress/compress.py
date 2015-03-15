#! /usr/bin/python3

import sys

if len(sys.argv) < 2:
    sys.exit('Usage: %s filename' % sys.argv[0])
    
fname=sys.argv[1]

all_values=[]
################################################################################
#RLE simple
with open(fname) as f:
  content = f.readlines()
  for line in content:
    for item in line[4:].split(" "):
      value_str=item.strip()[1:]
      if (len(value_str)>0):
        value=int(value_str,2)
        all_values.append(value)
      
print (len(all_values)*2)

previous_value=all_values[0]
previous_value_count=0
compressed_values=[]
for v in all_values:
  if (v==previous_value):
    previous_value_count+=1
  else:
    #print("For ",previous_value,": ",previous_value_count," times")
    compressed_values.append( (previous_value_count,previous_value) )
    previous_value_count=1
    previous_value=v

print(len(compressed_values)*3)

################################################################################
#2 RLE en parallele
all_values_Hi=[]
all_values_Lo=[]
for v in all_values:
  all_values_Hi.append( int(v/256) )
  all_values_Lo.append( int(v%256) )

previous_value=all_values[0]
previous_value_count=0
compressed_values_Hi=[]
for v in all_values_Hi:
  if (v==previous_value):
    previous_value_count+=1
  else:
    #print("For ",previous_value,": ",previous_value_count," times")
    compressed_values_Hi.append( (previous_value_count,previous_value) )
    previous_value_count=1
    previous_value=v

previous_value=all_values[0]
previous_value_count=0
compressed_values_Lo=[]
for v in all_values_Lo:
  if (v==previous_value):
    previous_value_count+=1
  else:
    #print("For ",previous_value,": ",previous_value_count," times")
    compressed_values_Lo.append( (previous_value_count,previous_value) )
    previous_value_count=1
    previous_value=v

print(len(compressed_values_Hi)*2+len(compressed_values_Lo)*2)

################################################################################
#on pourrait compresser le niveau 5  en colonnes, et ajouter un index des
#adresses de debut de colonne


################################################################################
#RLE avec series d'uniques
      
previous_value=all_values[0]
previous_value_count=0
compressed_values=[]
previous_uncompressed_values=[]
previous_uncompressed_values_count=0
for v in all_values:
  if (v==previous_value):
    previous_value_count+=1
    if (previous_value_count>1)and(previous_uncompressed_values_count>0):
      compressed_values.append(previous_uncompressed_values_count+128)
      compressed_values+=previous_uncompressed_values
      previous_uncompressed_values=[]
      previous_uncompressed_values_count=0
    if (previous_value_count>125):
      compressed_values.append( previous_value_count)
      compressed_values.append(previous_value)
      previous_value_count=0
  else:
    if (previous_value_count==1):
      previous_uncompressed_values.append(previous_value)
      previous_uncompressed_values_count+=1
    else:
      compressed_values.append( previous_value_count)
      compressed_values.append(previous_value)
    previous_value_count=1
    previous_value=v
print(compressed_values)
print(len(compressed_values)*1.5)
