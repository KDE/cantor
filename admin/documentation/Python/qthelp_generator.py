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

#index = open('./index.hhk', 'r')

# QtHelp files
qhp = open('./help.qhp', 'w')
qhcp = open('./help.qhcp', 'w')

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
            <section title="Python 3.8.4 Documentation" ref="python-3.8.4-docs-html/index.html">
                
                <section title="Parts of the documentation:" ref="python-3.8.4-docs-html/index.html">
                    <section title="What's new in Python 3.8?" ref="python-3.8.4-docs-html/whatsnew/3.8.html"></section>
                    <section title="Tutorial" ref="python-3.8.4-docs-html/tutorial/index.html"></section>
                    <section title="Library Reference" ref="python-3.8.4-docs-html/library/index.html"></section>
                    <section title="Language Reference" ref="python-3.8.4-docs-html/reference/index.html"></section>
                    <section title="Python Setup and Usage" ref="python-3.8.4-docs-html/using/index.html"></section>
                    <section title="Python HOWTOs" ref="python-3.8.4-docs-html/howto/index.html"></section>
                    <section title="Installing Python Modules" ref="python-3.8.4-docs-html/installing/index.html"></section>
                    <section title="Distributing Python Modules" ref="python-3.8.4-docs-html/distributing/index.html"></section>
                    <section title="Extending and Embedding" ref="python-3.8.4-docs-html/extending/index.html"></section>
                    <section title="Python/C API" ref="python-3.8.4-docs-html/c-api/index.html"></section>
                    <section title="FAQs" ref="python-3.8.4-docs-html/faq/index.html"></section>
                </section>

                <section title="Indices and tables:" ref="python-3.8.4-docs-html/index.html">
                    <section title="Global Module Index" ref="python-3.8.4-docs-html/py-modindex.html"></section>
                    <section title="General Index" ref="python-3.8.4-docs-html/genindex.html"></section>
                    <section title="Glossary" ref="python-3.8.4-docs-html/glossary.html"></section>
                    <section title="Search page" ref="python-3.8.4-docs-html/search.html"></section>
                    <section title="Complete Table of Contents" ref="python-3.8.4-docs-html/contents.html"></section>
                </section>

            </section>
        </toc>\n
        <keywords>""")

## write code for the keywords section


# write the tail
qhp.writelines("""</keywords>
            <files>
            <file>python-3.8.4-docs-html/*</file>
            <file>python-3.8.4-docs-html/_images/*</file>
            <file>python-3.8.4-docs-html/_sources/*</file>
            <file>python-3.8.4-docs-html/_static/*</file>
            <file>python-3.8.4-docs-html/c-api/*</file>
            <file>python-3.8.4-docs-html/distributing/*</file>
            <file>python-3.8.4-docs-html/distutils/*</file>
            <file>python-3.8.4-docs-html/extending/*</file>
            <file>python-3.8.4-docs-html/faq/*</file>
            <file>python-3.8.4-docs-html/howto/*</file>
            <file>python-3.8.4-docs-html/install/*</file>
            <file>python-3.8.4-docs-html/installing/*</file>
            <file>python-3.8.4-docs-html/library/*</file>
            <file>python-3.8.4-docs-html/reference/*</file>
            <file>python-3.8.4-docs-html/tutorial/*</file>
            <file>python-3.8.4-docs-html/using/*</file>
            <file>python-3.8.4-docs-html/whatsnew/*</file>
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

#index.close()
qhp.close()
qhcp.close()
