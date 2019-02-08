#!/usr/bin/Rscript
#
# script taken from RInside library
#
# This owes a lot to littler.R  in the littler sources

ExcludeVars <- c("R_SESSION_TMPDIR","R_HISTFILE")
IncludeVars <- Sys.getenv()
IncludeVars <- IncludeVars[grep("^R_",names(IncludeVars),perl=TRUE)]
cat("    const char *R_VARS[] = {\n")
for (i in 1:length(IncludeVars)){
	if (names(IncludeVars)[i] %in% ExcludeVars)
		next
	IncludeVars[i] <- gsub("\\\\",'/',IncludeVars[i])
	cat('        "',names(IncludeVars)[i],'","',IncludeVars[i],'",\n',sep='')
}
cat("        nullptr\n    };\n")
