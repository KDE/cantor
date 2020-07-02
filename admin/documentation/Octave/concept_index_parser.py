# script to parse Concept-Index.html and collect all the keywords into index.txt

from bs4 import BeautifulSoup

def is_ascii(s):
    return all(ord(c) < 128 for c in s)

fp = open('./Concept-Index.html', 'r')
fp2 = open('./index.txt', 'a')

html = fp.read()
soup = BeautifulSoup(html, features='html.parser')

for i in soup.find_all('tr'):
    link = i.findChildren()[1].findChild()
    if link is not None:
        if(is_ascii(link.text)):
           fp2.write('<keyword name = "{}" ref = "{}"/>\n'.format(link.text, link['href']))

fp.close()
fp2.close()
