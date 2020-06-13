import os

files = os.listdir('.')

for fi in files:
    if fi.endswith('.html'):

        # open file and reach to the 62 line
        fp = open(fi, 'r')

        lines_list = fp.readlines()
        lines_list.insert(61, '<link rel="stylesheet" type="text/css" href="./main.css">')

        fp.close()

        # link the css file
        fp = open(fi, 'w')
        fp.writelines(lines_list)
        fp.close()
