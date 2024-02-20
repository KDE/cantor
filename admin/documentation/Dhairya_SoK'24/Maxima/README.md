### Steps for adding custom style to the Maxima documentation

Add your custom styles to `main.css`. To apply custom them, place the python script named `qthelp_generator.py` in the same location as that of all the Maxima's documentation's HTML files. Then run the script. It will inject code into HTML files to link them to `main.css` file.