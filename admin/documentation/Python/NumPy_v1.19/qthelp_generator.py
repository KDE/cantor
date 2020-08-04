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
import re
from bs4 import BeautifulSoup

# QtHelp files
qhp = open('./help.qhp', 'w')
qhcp = open('./help.qhcp', 'w')

index = open("genindex.html", "r")

#######################################
#code for generation of QtHelp files##
######################################

# populate qhp file with headers and table of contents
qhp.writelines("""<?xml version="1.0" encoding="UTF-8"?>
<QtHelpProject version="1.0">
    <namespace>org.kde.numpy.1.19</namespace>

    <virtualFolder>doc</virtualFolder>
    
    <customFilter name="NumPy">
        <filterAttribute>NumPy Documentation</filterAttribute>
        <filterAttribute>1.19</filterAttribute>
    </customFilter>
    
    <filterSection>
        <toc>
            <section title="NumPy v1.19 Manual" ref="index.html">
                <section title="For users:" ref="index.html">
                    <section title="Setting Up" ref="user/setting-up.html"></section>
                    <section title="Quickstart Tutorial" ref="user/quickstart.html"></section>
                    <section title="Absolute Beginners Tutorial" ref="user/absolute_beginners.html"></section>
                    <section title="Tutorials" ref="user/tutorials_index.html"></section>
                    <section title="How Tos" ref="user/howtos_index.html"></section>
                    <section title="NumPy API Reference" ref="reference/index.html"></section>
                    <section title="Explanations" ref="user/explanations_index.html"></section>
                    <section title="F2Py Guide" ref="f2py/index.html"></section>
                    <section title="Glossary" ref="glossary.html"></section>
                </section>
            </section>
        </toc>\n
        <keywords>""")

# code to write keywords to qhp file
html = index.read()
soup = BeautifulSoup(html, features='html.parser')

for i in soup.find_all('a'):
    # replace the characters which produces error while qhcp file
    keyword = i.text.replace("<", "").replace("&", "")
    keyword = re.sub(r" ?\([^)]+\)", "", keyword)
    link = i['href']
    
    if keyword != "":
        line = '<keyword name = "{}" ref = "{}"/>\n'.format(keyword, link)
        qhp.write(line)

# write the tail
qhp.writelines("""</keywords>
            <files>
            <file>./*</file>
            <file>_static/*</file>
            <file>_static/img/*</file>
            <file>_images/*</file>
            <file>_images/math/*</file>
            <file>_sources/*</file>
            <file>dev/*</file>
            <file>dev/conducct/*</file>
            <file>dev/gitwash/*</file>
            <file>dev/gitwash/governance/*</file>
            <file>docs/*</file>
            <file>f2py/*</file>
            <file>reference/*</file>
            <file>reference/c-api/*</file>
            <file>reference/dist-utils/*</file>
            <file>reference/generated/*</file>
            <file>reference/random/*</file>
            <file>release/*</file>
            <file>user/*</file>
            <file>user/plots/*</file>
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
