# Variable cleaning script
for name in names(Main)[4:end]
    if name == :__originalSTDOUT__ || name == :__originalSTDERR__
        continue
    end
    var_info = summary(eval(name))
    if var_info == "Function" || var_info == "Module"
        continue
    end
    @eval (($name) = "%1")
end
