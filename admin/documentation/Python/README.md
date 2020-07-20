### Steps to generate `qhp` and `qhcp`
Copy the file named `qthelp_generator.py` to the location where the `Python` HTML files exists. Then simply run the command `python qthelp_generator.py`. This script does the task of extracting the keywords from `index.hhk` and generation of QtHelp files named `qhp` and `qhcp`.
NOTE: Copy the script named `qthelp_generator.py` to the parent directory `python-3.8.4-docs-html`. Official python documentation can be downloaded from https://docs.python.org/3/download.html.

### Creation of `qhc` and `qch`
Use the following command to generate the above said files.

    qhelpgenerator help.qhcp -o help.qhc
