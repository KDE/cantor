### Steps to generate `qhp` and `qhcp`
Copy the file named `qthelp_generator.py` to the location where the `Maxima` HTML files exists (maxima-website/docs/manual/). Then simply run the command `python qthelp_generator.py`. This script is all-in-one. It extracts the keywords from `maxima-website/docs/manual/maxima_363.html` and generation of QtHelp files named `qhp` and `qhcp`.

### Creation of `qhc` and `qch`
Use the following command to generate the above said files.

    qhelpgenerator help.qhcp -o help.qhc
