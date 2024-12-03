### Steps to generate `qhp` and `qhcp`
* Copy the file named `qthelp_generator.py` and custom style sheets named `main.css` to the location where the `Maxima` HTML files exists (maxima-website/docs/manual/)
* Then simply run the command `python qthelp_generator.py`. This script is all-in-one. It does the task of adding custom stylesheets defined in `main.css`, extracting the keywords from `index.hhk` and generation of QtHelp files named `qhp` and `qhcp`.
NOTE: store the `index.hhk`, `main.css` in the same folder as that of the `Maxima` documentation and the script should be placed there as well.

### Creation of `qhc` and `qch`
Use the following command to generate the above said files.

    qhelpgenerator help.qhcp -o help.qhc

### Steps for adding custom style to the Maxima documentation
Add your custom styles to `main.css`. To apply custom them, place the python script named `qthelp_generator.py` in the same location as that of all the Maxima's documentation's HTML files. Then run the script. It will inject code into HTML files to link them to `main.css` file.



### Steps for adding custom style to the Maxima documentation

1. With the help of `main.css` in the folder one can customise the documetation by modifying it.

2. To link the `main.css` with the HTML file copy the `script.py` file in the folder and run it.
