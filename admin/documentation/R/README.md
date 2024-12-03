## The manual "An Introduction to R"
This manual is part of the official release of R and is available in the the tgz file under doc/manual or in /usr/doc after the installations.
It's also available online on https://cran.r-project.org/doc/manuals/.

### Steps to generate the QtDoc files:
- copy R-intro.html and images/ sub-folder from R into a working directory
- copy `main.css` and `apply_css.py` from the parent folder into the sub-folder
- modify main.css by adding the following code:

```css
.calibre1 {
  font-size: 1em;
}

.top-level-extent {
  margin: 20px;
  background-color: #fff;
  padding: 20px;
  border-radius: 5px;
}
```
- execute `apply_css.py` to modify the html files
- copy help.qhcp and help.qhp from the sub-folder for the relevant version of R here into the same working directory
- execute qhelpgenerator help.qhcp -o help.qhc to generate the qhc file
