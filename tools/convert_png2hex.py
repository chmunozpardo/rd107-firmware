import imageio
import numpy as np
import matplotlib
matplotlib.use('Qt5Agg')
import matplotlib.pyplot as plt

name = 'button'
im = np.array(imageio.imread(name + '.png'))

savefile = name + '.c'

(x,y,z) = im.shape

colors = set([])

threshold = 32

min_x = x;
max_x = 0;
min_y = y;
max_y = 0;

print(x,y,z)

for i in range(x):
    for j in range(y):
        if not (im[i,j] == [0]*4).all():
            if(im[i,j,3] > threshold):
                colors.add('[255 0 0]')

output = {}
boundaries = {}
for color in colors:
    output[color] = np.zeros((x,y))
    boundaries[color] = {"max_y":0}

print(colors)
for i in range(x):
    for j in range(y):
        if not (im[i,j] == [0]*4).all():
            if(im[i,j,3] > threshold):
                color = '[255 0 0]'
                if color in colors:
                    if i < min_x:
                        min_x = i
                    if i > max_x:
                        max_x = i
                    if j < min_y:
                        min_y = j
                    if j > max_y:
                        max_y = j
                    output[color][i,j] = 1

print(min_x, min_y, max_x, max_y)

for color in colors:
    f = open(savefile, "w")
    string = 0

    dif_x = max_x - min_x + 1
    dif_y = max_y - min_y + 1
    f.write("const uint8_t " + name + "_Sign_Table [] =\n{\n")
    for i in range(dif_x):
        string = 0
        f.write("    ")
        for j in range(dif_y):
            if output[color][i+min_x,j+min_y] == 1:
                string += (1 << (-j-1)%8)
            if j%8 == 7 or j == dif_y-1:
                f.write("0x%02X,"%(string))
                string = 0
        f.write("\n")

    f.write("};\n\n")
    for c in "[]":
        color = color.replace(c, '')
    color = color.split()
    print(color)
    f.write("sFONT " + name +"_Sign =\n"
        "{"
        "\n    "+name+"_Sign_Table,"
        "\n    " + str(dif_y) + ", /* Width */"
        "\n    " + str(dif_x) + ", /* Height */"
    "\n};")
    f.write(hex((int(color[2]) >> 3) + ((int(color[1]) >> 2) << 5) + ((int(color[0]) >> 3) << 11)) + "\n")
    f.close()