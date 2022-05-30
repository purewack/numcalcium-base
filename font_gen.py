img_name = "fontsmall.png"
out_name = "fontsmall"
byte_size = 8
char_spacing = 8


from PIL import Image
import numpy

img = Image.open(img_name)
data = numpy.asarray(img.getdata())

byte = 0
w = img.size[0]
h = img.size[1]

fname = out_name + ".h"
with open(fname, 'w') as f:
    f.write("#pragma once \n\n")
    f.write("uint8_t " + out_name + "_spacing = " + str(byte_size) + ";\n")
    f.write("uint" + str(byte_size) + "_t " + out_name + "_none [" + str(8) + "] = {\n")
    for x in range(8):
        for y in range(h):
            if(data[x + (y*w)][0] > 0):
                byte |= (1<<y)
        
        end =  ",\n" if (x<8-1) else " \n"
        f.write("\t" + hex(byte) + end)
        byte = 0
    f.write("};\n\n")

    f.write("uint" + str(byte_size) + "_t " + out_name + " [" + str(w-8) + "] = {\n")
    for x in range(8,w):
        for y in range(h):
            if(data[x + (y*w)][0] > 0):
                byte |= (1<<y)
        
        end =  ",\n" if (x<w-1) else " \n"
        f.write("\t" + hex(byte) + end)
        byte = 0
    f.write("};\n")