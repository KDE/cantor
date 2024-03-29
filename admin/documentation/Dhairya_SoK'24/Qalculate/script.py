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

#######################################
#code for generation of QtHelp files##
######################################

qhp = open('./help.qhp', 'w')
qhcp = open('./help.qhcp', 'w')

qhp.writelines("""<?xml version="1.0" encoding="UTF-8"?>
<QtHelpProject version="1.0">
    <namespace>org.kde.qalculate</namespace>
    
    <virtualFolder>doc</virtualFolder>

    <customFilter name="Qalculate">
        <filterAttribute>NumPy Documentation</filterAttribute>
        <filterAttribute>1.19</filterAttribute>
    </customFilter>
    
    <filterSection>
        <toc>
            <section title="Qalculate! Manual v4.9.0" ref="index.html">

                <section title="QALC man page" ref="qalc.html"></section>

                <section title="Chapter 1. Introduction" ref="qalculate-introduction.html"></section>

                <section title="Chapter 2. Differences in the Qt user interface" ref="qalculate-qt.html"></section>

                <section title="Chapter 3. User Interface" ref="qalculate-user-interface.html"></section>

                <section title="Chapter 4. Expressions" ref="qalculate-expressions.html"></section>
    
                <section title="Chapter 5. Calculator Modes" ref="qalculate-mode.html"></section>
    
                <section title="Chapter 6. Propagation of Uncertainty and Interval Arithmetic" ref="qalculate-interval-arithmetic.html"></section>

                <section title="Chapter 7. Variables" ref="qalculate-variables.html"></section>

                <section title="Chapter 8. Functions" ref="qalculate-functions.html"></section>

                <section title="Chapter 9. Units" ref="qalculate-units.html"></section>

                <section title="Chapter 10. Plotting" ref="qalculate-plotting.html"></section>

                <section title="Appendix A. Function List" ref="qalculate-definitions-functions.html"></section>
                
                <section title="Appendix B. Variable List" ref="qalculate-definitions-variables.html"></section>

                <section title="Appendix C. Unit List" ref="qalculate-definitions-units.html"></section>

                <section title="Appendix D. Example expressions" ref="qalculate-examples.html"></section>

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
            <file>*.html</file>
            <file>figures/*.svg</file>
            <file>figures/*.png</file>
            <file>main.css</file>
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
