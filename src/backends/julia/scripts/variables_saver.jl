# Variable saving script
import JLD
JLD.jldopen("%1", "w") do file
    for name in names(Main)[4:end]
        if name == :__originalSTDOUT__ || name == :__originalSTDERR__
            continue
        end
        var_info = summary(eval(name))
        if var_info == "Function" || var_info == "Module"
            continue
        end
        JLD.write(file, repr(name)[2:end], eval(name))
    end
end
