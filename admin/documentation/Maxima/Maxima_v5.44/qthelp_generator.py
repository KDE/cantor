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
# and then generate QtHelp files using the keywords generated
# This script also adds custom styles to the html pages

import os
import re
from bs4 import BeautifulSoup

files = os.listdir('.')

index = open('./maxima_363.html', 'r')
# QtHelp files
qhp = open('./help.qhp', 'w')
qhcp = open('./help.qhcp', 'w')

html = index.read()
soup = BeautifulSoup(html, features='html.parser')

title = soup.find('title').text

#######################################
#code for generation of QtHelp files##
######################################

# populate qhp file with headers and table of contents
qhp.writelines("""<?xml version="1.0" encoding="UTF-8"?>
<QtHelpProject version="1.0">
    <namespace>org.kde.maxima.5.42</namespace>

    <virtualFolder>doc</virtualFolder>

    <customFilter name="Maxima">
        <filterAttribute>Maxima Documentation</filterAttribute>
        <filterAttribute>Maxima 5.44</filterAttribute>
    </customFilter>

    <filterSection>
        <toc>
            <section title="Maxima Documentation 5.44" ref="maxima_0.html">

                <section title="Maxima Infrastructure" ref="maxima.html#SEC1">
                    <section title="Introduction to Maxima" ref="maxima.html#Introduction-to-Maxima"></section>
                    <section title="Bug Detection and Reporting" ref="maxima_1.html#Bug-Detection-and-Reporting"></section>
                    <section title="Help" ref="maxima_3.html#Help"></section>
                    <section title="Command Line" ref="maxima_6.html#Command-Line"></section>
                    <section title="Data Types and Structures" ref="maxima_10.html#Data-Types-and-Structures"></section>
                    <section title="Expressions" ref="maxima_28.html#Expressions"></section>
                    <section title="Operators" ref="maxima_34.html#Operators"></section>
                    <section title="Evaluation" ref="maxima_42.html#Evaluation"></section>
                    <section title="Simplification" ref="maxima_44.html#Simplification"></section>
                    <section title="Mathematical Functions" ref="maxima_47.html#Mathematical-Functions"></section>
                    <section title="Maximas Database" ref="maxima_56.html#Maxima_0027s-Database"></section>
                    <section title="Plotting" ref="maxima_61.html#Plotting"></section>
                    <section title="File Input and Output" ref="maxima_68.html#File-Input-and-Output"></section>
                </section>


                <section title="Support for specific areas of mathematics" ref="maxima_0.html">
                    <section title="Polynomials" ref="maxima_74.html#Polynomials"></section>
                    <section title="Special Functions" ref="maxima_77.html#Special-Functions"></section>
                    <section title="Elliptic Functions" ref="maxima_88.html#Elliptic-Functions"></section>
                    <section title="Limits" ref="maxima_92.html#Limits"></section>
                    <section title="Differentiation" ref="maxima_94.html#Differentiation"></section>
                    <section title="Integration" ref="maxima_96.html#Integration"></section>
                    <section title="Equations" ref="maxima_101.html#Equations"></section>
                    <section title="Differential Equations" ref="maxima_103.html#Differential-Equations"></section>
                    <section title="Numerical" ref="maxima_106.html#Numerical"></section>
                    <section title="Matrices and Linear Algebra" ref="maxima_112.html#Matrices-and-Linear-Algebra"></section>
                    <section title="Affine" ref="maxima_119.html#Affine"></section>
                    <section title="itensor" ref="maxima_122.html#itensor"></section>
                    <section title="ctensor" ref="maxima_125.html#ctensor"></section>
                    <section title="atensor" ref="maxima_128.html#atensor"></section>
                    <section title="Sums, Products and Series" ref="maxima_131.html#Sums-Products-and-Series"></section>
                    <section title="Number Theory" ref="maxima_138.html#Number-Theory"></section>
                    <section title="Symmetrics" ref="maxima_140.html#Symmetries"></section>
                    <section title="Groups" ref="maxima_143.html#Groups"></section>
                </section>


                <section title="Advanced facilities and programming" ref="maxima_0.html">
                    <section title="Runtime Environment" ref="maxima_145.html#Runtime-Environment"></section>
                    <section title="Miscellaneous Options" ref="maxima_149.html#Miscellaneous-Options"></section>
                    <section title="Rules and Patterns" ref="maxima_153.html#Rules-and-Patterns"></section>
                    <section title="Sets" ref="maxima_156.html#Sets"></section>
                    <section title="Function Definition" ref="maxima_159.html#Function-Definition"></section>
                    <section title="Program Flow" ref="maxima_164.html#Program-Flow"></section>
                    <section title="Debugging" ref="maxima_169.html#Debugging"></section>
                </section>


                <section title="Additional packages" ref="maxima_0.html">
                    <section title="alt-display" ref="maxima_173.html#alt_002ddisplay_002dpkg"></section>
                    <section title="asympa" ref="maxima_176.html#asympa_002dpkg"></section>
                    <section title="augmented_lagrangian" ref="maxima_179.html#augmented_005flagrangian_002dpkg"></section>
                    <section title="Bernstein" ref="maxima_181.html#Bernstein_002dpkg"></section>
                    <section title="bitwise" ref="maxima_183.html#bitwise_002dpkg"></section>
                    <section title="bode" ref="maxima_185.html#bode_002dpkg"></section>
                    <section title="celine" ref="maxima_187.html#celine_002dpkg"></section>
                    <section title="clebsch_gordan" ref="maxima_189.html#clebsch_005fgordan_002dpkg"></section>
                    <section title="cobyla" ref="maxima_191.html#cobyla_002dpkg"></section>
                    <section title="combinatorics" ref="maxima_195.html#combinatorics_002dpkg"></section>
                    <section title="contrib_ode" ref="maxima_198.html#contrib_005fode_002dpkg"></section>
                    <section title="desciptive" ref="maxima_204.html#descriptive_002dpkg"></section>
                    <section title="diag" ref="maxima_209.html#diag_002dpkg"></section>
                    <section title="distrib" ref="maxima_211.html#distrib_002dpkg"></section>
                    <section title="draw" ref="maxima_215.html#draw_002dpkg"></section>
                    <section title="drawdf" ref="maxima_220.html#drawdf_002dpkg"></section>
                    <section title="dynamics" ref="maxima_223.html#dynamics_002dpkg"></section>
                    <section title="engineering-format" ref="maxima_227.html#engineering_002dformat_002dpkg"></section>
                    <section title="ezunits" ref="maxima_230.html#ezunits_002dpkg"></section>
                    <section title="f90" ref="maxima_234.html#f90_002dpkg"></section>
                    <section title="finance" ref="maxima_236.html#finance_002dpkg"></section>
                    <section title="fractals" ref="maxima_239.html#fractals_002dpkg"></section>
                    <section title="ggf" ref="maxima_245.html#ggf_002dpkg"></section>
                    <section title="graphs" ref="maxima_247.html#graphs_002dpkg"></section>
                    <section title="grobner" ref="maxima_250.html#grobner_002dpkg"></section>
                    <section title="impdiff" ref="maxima_253.html#impdiff_002dpkg"></section>
                    <section title="interpol" ref="maxima_255.html#interpol_002dpkg"></section>
                    <section title="lapack" ref="maxima_258.html#lapack_002dpkg"></section>
                    <section title="lbfgs" ref="maxima_261.html#lbfgs_002dpkg"></section>
                    <section title="lindstedt" ref="maxima_264.html#lindstedt_002dpkg"></section>
                    <section title="linearalgebra" ref="maxima_266.html#linearalgebra_002dpkg"></section>
                    <section title="lsquares" ref="maxima_269.html#lsquares_002dpkg"></section>
                    <section title="makeOrders" ref="maxima_275.html#makeOrders_002dpkg"></section>
                    <section title="minpack" ref="maxima_272.html#minpack_002dpkg"></section>
                    <section title="mnewton" ref="maxima_277.html#mnewton_002dpkg"></section>
                    <section title="numericalio" ref="maxima_280.html#numericalio_002dpkg"></section>
                    <section title="operatingsystem" ref="maxima_288.html#operatingsystem_002dpkg"></section>
                    <section title="opsubst" ref="maxima_293.html#opsubst_002dpkg"></section>
                    <section title="orthopoly" ref="maxima_295.html#orthopoly_002dpkg"></section>
                    <section title="ratpow" ref="maxima_302.html#ratpow_002dpkg"></section>
                    <section title="romberg" ref="maxima_304.html#romberg_002dpkg"></section>
                    <section title="simplex" ref="maxima_306.html#simplex_002dpkg"></section>
                    <section title="simplification" ref="maxima_309.html#simplification_002dpkg"></section>
                    <section title="solve_rec" ref="maxima_317.html#solve_005frec_002dpkg"></section>
                    <section title="stats" ref="maxima_320.html#stats_002dpkg"></section>
                    <section title="stirling" ref="maxima_325.html#stirling_002dpkg"></section>
                    <section title="stringproc" ref="maxima_327.html#stringproc_002dpkg"></section>
                    <section title="to_poly_solve" ref="maxima_333.html#to_005fpoly_005fsolve_002dpkg"></section>
                    <section title="unit" ref="maxima_335.html#unit_002dpkg"></section>
                    <section title="wrstcse" ref="maxima_338.html#wrstcse_002dpkg"></section>
                    <section title="zeilberger" ref="maxima_341.html#zeilberger_002dpkg"></section>
                </section>


                <section title="Understanding maxima's output" ref="maxima_0.html">
                    <section title="Error and Warning Messages" ref="maxima_344.html#Error-and-warning-messages5"></section>
                </section>

            </section>
        </toc>\n
        <keywords>""")

# read all the keywords from maxima_363.html and write them to qhp under <keywords> section
for i in soup.find_all('code'):
    keyword = i.text.replace("<", "")
    keyword = re.sub(r" ?\([^)]+\)", "", keyword)
    ref = i.find_parent('a')
    
    qhp.write('<keyword name = "{}" ref = "{}"/>\n'.format(keyword, ref['href']))

# write the tail
qhp.writelines("""</keywords>
            <files>
            <file>*.html</file>
            <file>figures/*.gif</file>
            <file>figures/*.png</file>
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

index.close()
qhp.close()
qhcp.close()
