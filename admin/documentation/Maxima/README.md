# 1. Creation of `qhp`
It stands for Qt Help Project. This is simiar to xml's file format. It contains the table of contents, indices, and references to the actual documentation files (*.html). This file is later passed to qhcp (Qt Help Collection File). This file contains various tags like table of contents, section tag for defining the actual documentation, keywords tag for defining the indices for the documentation, files tag for listing all the files required. [Click here for more details on qhp](https://doc.qt.io/qt-5/qthelpproject.html)

### Steps to extract the indices from `index.hhk` file to add them to `qhp`
`index.hhk` is an index file shipped with the Maxima documentation. To extract all the indices, run the python script named `index_parser.py` over the index file to get the keywords listed for `qhp` file. Copy and paste the `output.txt` file's content to `qhp` file under keywords section.


# 2. Creation of `qhcp`

It is an XML file that contains references to the compressed help files that should be included in the help collection. This file can be passed to the help generator for creating a `qhc` and `qch` files in one go. [Refer this link for more information](https://doc.qt.io/qt-5/qthelp-framework.html#qt-help-collection-project)

Use the following command to generate the above said files.

    qhelpgenerator doc.qhp -o doc.qch

# 3. Steps for adding custom style to the Maxima documentation
Add your custom styles to `main.css`. To apply custom them, place the python script named `css_injector.py` in the same location as that of all the Maxima's documentation's HTML files. Then run the script. It will inject code into HTML files to link them to `main.css` file.
