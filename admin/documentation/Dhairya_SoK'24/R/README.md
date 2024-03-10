### Steps for adding custom style to the Qalculate documentation

1. Copy and paste the `main.css` and `script.py` files in the R folder.

2. Add below mentioned CSS rules in the copied file.

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

3. Link this file with the HTML files of R Documentation.

4. This can be done by running the `script.py` file. 

5. Using this `main.css` file one can customize the R Documentation by adding custom css rules in it.
