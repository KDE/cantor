# script to parse index.hhk and generate .xml file

fp = open('./index.hhk', 'r')
fp2 = open('./output.txt', 'w')

for li in fp:
    if li.startswith('<li>'):

        line2 = fp.next()
        line3 = fp.next()

        if not line2:
            break
        else:
            ln2_lastindex = line2.rindex('"')
            ln3_lastindex = line3.rindex('"')

            name = line3[29:ln3_lastindex]
            ref = line2[30:ln2_lastindex]

            fp2.write('<keyword name = "{}" ref = "{}"/>\n'.format(name, ref))

fp.close()
fp2.close()
