# script to parse Function-Index.html and collect all the keywords into index.txt

from bs4 import BeautifulSoup

fp = open('./Function-Index.html', 'r')
fp2 = open('./index.txt', 'a')

html = fp.read()
soup = BeautifulSoup(html, features='html.parser')

for i in soup.find_all('code'):
    index = i.text
    ref = i.find_parent('a')

    fp2.write('<keyword name = "{}" ref = "{}"/>\n'.format(index, ref['href']))

fp.close()
fp2.close()
