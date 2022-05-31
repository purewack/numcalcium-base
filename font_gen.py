img_name = "fontsmall.png"
out_name = "fontsmall"
char_height = 8
char_width = 8


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
    f.write("uint8_t " + out_name + "_tall = " + str(char_height) + ";\n")
    f.write("uint8_t " + out_name + "_wide = " + str(char_width) + ";\n")

    f.write("uint" + str(char_height) + "_t " + out_name + "_data [" + str(w) + "] = {\n")
    for x in range(w):
        for y in range(h):
            if(data[x + (y*w)][0] > 0):
                byte |= (1<<y)
        
        end =  ",\n" if (x<w-1) else " \n"
        f.write("\t" + hex(byte) + end)
        byte = 0
    f.write("};\n")