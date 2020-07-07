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

# script to parse index.hhk and generate a txt file containing all the keywords
# and then genearte QtHelp files using the keywords generated
# This script also adds custom styles to the html pages

import os
from bs4 import BeautifulSoup

#######################################
#code for adding custom styles to html#
######################################
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


#####################
#open required files#
#####################
index = open('./index.hhk', 'r')

# QtHelp files
qhp = open('./help.qhp', 'w')
qhcp = open('./help.qhcp', 'w')

#######################################
#code for generation of QtHelp files##
######################################

# populate qhp file with headers and table of contents
qhp.writelines("""<?xml version="1.0" encoding="UTF-8"?>
<QtHelpProject version="1.0">
    <namespace>org.kde.maxima</namespace>

    <virtualFolder>doc</virtualFolder>

    <customFilter name="Maxima">
        <filterAttribute>Maxima Documentation</filterAttribute>
        <filterAttribute>5.42</filterAttribute>
    </customFilter>

    <filterSection>
        <toc>
            <section title="Maxima 5.42 Documentation" ref="maxima.html">

                <section title="Maxima Infrastructure" ref="maxima.html#SEC1">
                    <section title="Introduction to Maxima" ref="maxima_1.html"></section>
                    <section title="Bug Detection and Reporting" ref="maxima_2.html"></section>
                    <section title="Help" ref="maxima_3.html"></section>
                    <section title="Command Line" ref="maxima_4.html"></section>
                    <section title="Data Types and Structures" ref="maxima_5.html"></section>
                    <section title="Expressions" ref="maxima_6.html"></section>
                    <section title="Operators" ref="maxima_7.html"></section>
                    <section title="Evaluation" ref="maxima_8.html"></section>
                    <section title="Simplification" ref="maxima_9.html"></section>
                    <section title="Mathematical Functions" ref="maxima_10.html"></section>
                    <section title="Maximas Database" ref="maxima_11.html"></section>
                    <section title="Plotting" ref="maxima_12.html"></section>
                    <section title="File Input and Output" ref="maxima_13.html"></section>
                </section>


                <section title="Support for specific areas of mathematics" ref="maxima.html#SEC2">
                    <section title="Polynomials" ref="maxima_14.html"></section>
                    <section title="Special Functions" ref="maxima_15.html"></section>
                    <section title="Elliptic Functions" ref="maxima_16.html"></section>
                    <section title="Limits" ref="maxima_17.html"></section>
                    <section title="Differentiation" ref="maxima_18.html"></section>
                    <section title="Integration" ref="maxima_19.html"></section>
                    <section title="Equations" ref="maxima_20.html"></section>
                    <section title="Differential Equations" ref="maxima_21.html"></section>
                    <section title="Numerical" ref="maxima_22.html"></section>
                    <section title="Matrices and Linear Algebra" ref="maxima_23.html"></section>
                    <section title="Affine" ref="maxima_24.html"></section>
                    <section title="itensor" ref="maxima_25.html"></section>
                    <section title="ctensor" ref="maxima_26.html"></section>
                    <section title="atensor" ref="maxima_27.html"></section>
                    <section title="Sums, Products and Series" ref="maxima_28.html"></section>
                    <section title="Number Theory" ref="maxima_29.html"></section>
                    <section title="Symmetrics" ref="maxima_30.html"></section>
                    <section title="Groups" ref="maxima_31.html"></section>
                </section>


                <section title="Advanced facilities and programming" ref="maxima.html#SEC3">
                    <section title="Runtime Environment" ref="maxima_32.html"></section>
                    <section title="Miscellaneous Options" ref="maxima_33.html"></section>
                    <section title="Rules and Patterns" ref="maxima_34.html"></section>
                    <section title="Sets" ref="maxima_35.html"></section>
                    <section title="Function Definition" ref="maxima_36.html"></section>
                    <section title="Program Flow" ref="maxima_37.html"></section>
                    <section title="Debugging" ref="maxima_38.html"></section>
                </section>


                <section title="Additional packages" ref="maxima.html#SEC4">
                    <section title="alt-display" ref="maxima_39.html"></section>
                    <section title="asympa" ref="maxima_40.html"></section>
                    <section title="augmented_lagrangian" ref="maxima_41.html"></section>
                    <section title="Bernstein" ref="maxima_42.html"></section>
                    <section title="bitwise" ref="maxima_43.html"></section>
                    <section title="bode" ref="maxima_44.html"></section>
                    <section title="celine" ref="maxima_45.html"></section>
                    <section title="clebsch_gordan" ref="maxima_46.html"></section>
                    <section title="cobyla" ref="maxima_47.html"></section>
                    <section title="combinatorics" ref="maxima_48.html"></section>
                    <section title="contrib_ode" ref="maxima_49.html"></section>
                    <section title="desciptive" ref="maxima_50.html"></section>
                    <section title="diag" ref="maxima_51.html"></section>
                    <section title="distrib" ref="maxima_52.html"></section>
                    <section title="draw" ref="maxima_53.html"></section>
                    <section title="drawdf" ref="maxima_54.html"></section>
                    <section title="dynamics" ref="maxima_55.html"></section>
                    <section title="engineering-format" ref="maxima_56.html"></section>
                    <section title="ezunits" ref="maxima_57.html"></section>
                    <section title="f90" ref="maxima_58.html"></section>
                    <section title="finance" ref="maxima_59.html"></section>
                    <section title="fractals" ref="maxima_60.html"></section>
                    <section title="ggf" ref="maxima_61.html"></section>
                    <section title="graphs" ref="maxima_62.html"></section>
                    <section title="grobner" ref="maxima_63.html"></section>
                    <section title="impdiff" ref="maxima_64.html"></section>
                    <section title="interpol" ref="maxima_65.html"></section>
                    <section title="lapack" ref="maxima_66.html"></section>
                    <section title="lbfgs" ref="maxima_67.html"></section>
                    <section title="lindstedt" ref="maxima_68.html"></section>
                    <section title="linearalgebra" ref="maxima_69.html"></section>
                    <section title="lsquares" ref="maxima_70.html"></section>
                    <section title="makeOrders" ref="maxima_71.html"></section>
                    <section title="minpack" ref="maxima_72.html"></section>
                    <section title="mnewton" ref="maxima_73.html"></section>
                    <section title="numericalio" ref="maxima_74.html"></section>
                    <section title="operatingsystem" ref="maxima_75.html"></section>
                    <section title="opsubst" ref="maxima_76.html"></section>
                    <section title="orthopoly" ref="maxima_77.html"></section>
                    <section title="ratpow" ref="maxima_78.html"></section>
                    <section title="romberg" ref="maxima_79.html"></section>
                    <section title="simplex" ref="maxima_80.html"></section>
                    <section title="simplification" ref="maxima_81.html"></section>
                    <section title="solve_rec" ref="maxima_82.html"></section>
                    <section title="stats" ref="maxima_83.html"></section>
                    <section title="stirling" ref="maxima_84.html"></section>
                    <section title="stringproc" ref="maxima_85.html"></section>
                    <section title="to_poly_solve" ref="maxima_86.html"></section>
                    <section title="unit" ref="maxima_87.html"></section>
                    <section title="wrstcse" ref="maxima_88.html"></section>
                    <section title="zeilberger" ref="maxima_89.html"></section>
                </section>


                <section title="Understanding maxima's output" ref="maxima.html#SEC5">
                    <section title="Error and Warning Messages" ref="maxima_90.html"></section>
                </section>

            </section>
        </toc>\n
        <keywords>""")

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



# this way does not work, because somehow beautifulsoup is unable to escape &gt and &gt;< special symbols
# html = index.read()
# soup = BeautifulSoup(html, features='html.parser')

# for i in soup.find_all('object'):
#     attributes = i.findChildren()
#     link = attributes[0]['value']
#     keyword = attributes[1]['value']
#     repr(keyword)

#     if link != 'maxima_7.html#IDX227' and link != 'maxima_7.html#IDX227':
#         qhp.write('<keyword name = "{}" ref = "{}"/>\n'.format(keyword, link))

# read all the keywords from index.hhk and write them to qhp under <keywords> section
for li in index:
    if li.startswith('<li>'):

        line2 = index.next()
        line3 = index.next()

        if not line2:
            break
        else:
            ln2_lastindex = line2.rindex('"')
            ln3_lastindex = line3.rindex('"')

            name = line3[29:ln3_lastindex]
            ref = line2[30:ln2_lastindex]

            qhp.write('<keyword name = "{}" ref = "{}"/>\n'.format(name, ref))


# write the tail
qhp.writelines("""</keywords>
            <files>
            <file>*.html</file>
            <file>figures/*.gif</file>
            <file>figures/*.png</file>
            <file>main.css</file>
        </files>
    </filterSection>

</QtHelpProject> """)

##############################################################
#qhp, qhcp input files are generate, now generate output files
#############################################################

stream = os.popen('qhelpgenerator help.qhcp -o help.qhc')

index.close()
qhp.close()
qhcp.close()
