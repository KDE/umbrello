#! /bin/bash

echo Converting from XMI to docbook...
java -cp /usr/share/java/xalan-j2-2.6.0.jar org.apache.xalan.xslt.Process -xml -in $1.xmi -xsl /home/gael/Logiciels/kde3.5-svn/kdesdk/umbrello.withdocgen/umbrello/docgenerators/xmi2docbook.xsl -out $1.docbook 

echo Converting from docbook to XHTML...
java -cp /usr/share/java/xalan-j2-2.6.0.jar org.apache.xalan.xslt.Process -in $1.docbook -xsl /home/gael/Logiciels/kde3.5-svn/kdesdk/umbrello.withdocgen/umbrello/docgenerators/docbook2xhtml.xsl -out $1.html -html

echo done.
