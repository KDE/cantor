### Steps to generate `qhp` and `qhcp`
Copy the file named `qthelp_generator.py` to the location where the `NumPy` HTML files exists. Then simply run the command `python qthelp_generator.py`. This script does the task of extracting the keywords from `Numpy_v1.19/genindex.html` and generation of QtHelp files named `qhp` and `qhcp`.
NOTE: Copy the script named `qthelp_generator.py` inside the directory `Numpy_v1.19/`. Official python documentation can be downloaded from https://numpy.org/doc/.

### Creation of `qhc` and `qch`
Use the following command to generate the above said files.

    qhelpgenerator help.qhcp -o help.qhc
