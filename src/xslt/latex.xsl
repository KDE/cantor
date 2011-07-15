<xsl:stylesheet version = "1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method = "text"/>
<xsl:strip-space elements = "*"/>

<xsl:template match="Worksheet">
<xsl:text>\documentclass[a4paper,10pt,fleqn]{article}

\usepackage{fullpage}
\usepackage{graphicx}
\usepackage[utf8]{inputenc}
\usepackage{amsmath,amssymb}

\begin{document}
</xsl:text>
<xsl:apply-templates/>
<xsl:text>\end{document}&#xA;</xsl:text>
</xsl:template>

<xsl:template match="Result">
<xsl:apply-templates/>
<xsl:text>&#xA;</xsl:text>
</xsl:template>

<xsl:template match="Command">
<xsl:text>\begin{verbatim}&#xA;</xsl:text>
<xsl:apply-templates/>
<xsl:text>&#xA;\end{verbatim}&#xA;</xsl:text>
</xsl:template>

<xsl:template match="PageBreak">
<xsl:apply-templates/>
<xsl:text>&#xA;\newpage{}&#xA;</xsl:text>
</xsl:template>

<xsl:template match="Image">
<xsl:if test="Path != ''">
<xsl:text>\begin{center}
\includegraphics</xsl:text>
<xsl:value-of select="LatexSizeString" />
<xsl:text>{</xsl:text>
<xsl:value-of select="Path" />
<xsl:text>}&#xA;\end{center}&#xA;</xsl:text>
</xsl:if>
</xsl:template>

<xsl:template match="p">
<xsl:apply-templates/>
<xsl:text>&#xA;</xsl:text>
</xsl:template>

<xsl:template match="ul">
<xsl:text>\begin{itemize}&#xA;</xsl:text>
<xsl:apply-templates/>
<xsl:text>\end{itemize}&#xA;</xsl:text>
</xsl:template>

<xsl:template match="ol">
<xsl:text>\begin{enumerate}&#xA;</xsl:text>
<xsl:apply-templates/>
<xsl:text>\end{enumerate}&#xA;</xsl:text>
</xsl:template>

<xsl:template match="li">
<xsl:text>  \item </xsl:text>
<xsl:apply-templates/>
<xsl:text>&#xA;</xsl:text>
</xsl:template>

<xsl:template match="span">
    <xsl:text> </xsl:text>
        <xsl:apply-templates/>
    <xsl:text> </xsl:text>
</xsl:template>

<xsl:template match="text()">
    <xsl:value-of select="normalize-space()"/>
</xsl:template>

</xsl:stylesheet>
