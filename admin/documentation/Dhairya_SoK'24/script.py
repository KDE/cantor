import os
from bs4 import BeautifulSoup

css_link = BeautifulSoup('<link rel="stylesheet" type="text/css" href="main.css">', 'html.parser')

for filename in os.listdir('.'):
    if filename.endswith('.html'):
        with open(filename, 'r+') as f:
            soup = BeautifulSoup(f, 'html.parser')
            head = soup.head
            head.append(css_link)
            f.seek(0)
            f.write(str(soup))
            f.truncate()