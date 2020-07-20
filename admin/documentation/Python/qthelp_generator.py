#
#     This program is free software; you can redistribute it and/or
#     modify it under the terms of the GNU General Public License
#     as published by the Free Software Foundation; either version 2
#     of the License, or (at your option) any later version.

#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.

#     You should have received a copy of the GNU General Public License
#     along with this program; if not, write to the Free Software
#     Foundation, Inc., 51 Franklin Street, Fifth Floor,
#     Boston, MA  02110-1301, USA.

#     ---
#     Copyright (C) 2020 Shubham <aryan100jangid@gmail.com>
#

# Script to parse index and generate a txt file containing all the keywords
# and then generate QtHelp files using the keywords generated

import os
from bs4 import BeautifulSoup

# QtHelp files
qhp = open('./help.qhp', 'w')
qhcp = open('./help.qhcp', 'w')

index = open("genindex-all.html", "r")
index2 = open("genindex-_.html", "r")

#######################################
#code for generation of QtHelp files##
######################################

# populate qhp file with headers and table of contents
qhp.writelines("""<?xml version="1.0" encoding="UTF-8"?>
<QtHelpProject version="1.0">
    <namespace>org.kde.python.3.8.4</namespace>

    <virtualFolder>doc</virtualFolder>

    <customFilter name="Python">
        <filterAttribute>Python Documentation</filterAttribute>
        <filterAttribute>3.8.4</filterAttribute>
    </customFilter>

    <filterSection>
        <toc>
            <section title="Python 3.8.4 Documentation" ref="index.html">

                <section title="Parts of the documentation:" ref="index.html">
                    <section title="What's new in Python 3.8?" ref="whatsnew/3.8.html"></section>
                    <section title="Tutorial" ref="tutorial/index.html"></section>
                    <section title="Library Reference" ref="library/index.html"></section>
                    <section title="Language Reference" ref="reference/index.html"></section>
                    <section title="Python Setup and Usage" ref="using/index.html"></section>
                    <section title="Python HOWTOs" ref="howto/index.html"></section>
                    <section title="Installing Python Modules" ref="installing/index.html"></section>
                    <section title="Distributing Python Modules" ref="distributing/index.html"></section>
                    <section title="Extending and Embedding" ref="extending/index.html"></section>
                    <section title="Python/C API" ref="c-api/index.html"></section>
                    <section title="FAQs" ref="faq/index.html"></section>
                </section>

                <section title="Indices and tables:" ref="index.html">
                    <section title="Global Module Index" ref="py-modindex.html"></section>
                    <section title="General Index" ref="genindex.html"></section>
                    <section title="Glossary" ref="glossary.html"></section>
                    <section title="Search page" ref="search.html"></section>
                    <section title="Complete Table of Contents" ref="contents.html"></section>
                </section>

            </section>
        </toc>\n
        <keywords>""")

# code to write keywords to qhp file
html = index.read()
soup = BeautifulSoup(html, features='html.parser')

for i in soup.find_all('a'):
    ## replace the characters which produces error while qhcp file
    keyword = i.text.replace("<", "").replace("&", "")
    link = i['href']

    line = '<keyword name = "{}" ref = "{}"/>\n'.format(keyword, link)
    qhp.write(line)

html2 = index2.read()
soup2 = BeautifulSoup(html, features='html.parser')

for i in soup2.find_all('a'):
    ## replace the characters which produces error while qhcp file
    keyword = i.text.replace("<", "").replace("&", "")
    link = i['href']

    line = '<keyword name = "{}" ref = "{}"/>\n'.format(keyword, link)
    qhp.write(line)

# write the tail
qhp.writelines("""</keywords>
            <files>
            <file>./*</file>
            <file>_images/*</file>
            <file>_sources/*</file>
            <file>_static/*</file>
            <file>c-api/*</file>
            <file>distributing/*</file>
            <file>distutils/*</file>
            <file>extending/*</file>
            <file>faq/*</file>
            <file>howto/*</file>
            <file>install/*</file>
            <file>installing/*</file>
            <file>library/*</file>
            <file>reference/*</file>
            <file>tutorial/*</file>
            <file>using/*</file>
            <file>whatsnew/*</file>
        </files>
    </filterSection>
</QtHelpProject> """)

# populate qhcp file
qhcp.writelines("""<?xml version="1.0" encoding="utf-8" ?>
            <QHelpCollectionProject version="1.0">

                <docFiles>
                    <generate>
                        <file>
                            <input>help.qhp</input>
                            <output>help.qch</output>
                        </file>
                    </generate>

                    <register>
                        <file>help.qch</file>
                    </register>
                </docFiles>

            </QHelpCollectionProject>""")

##############################################################
#qhp, qhcp input files are generate, now generate output files
#############################################################

stream = os.popen('qhelpgenerator help.qhcp -o help.qhc')

qhp.close()
qhcp.close()
index.close()
index2.close()
