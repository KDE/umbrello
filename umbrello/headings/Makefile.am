####### kdevelop will overwrite this part!!! (begin)##########


EXTRA_DIST = heading.java heading.h heading.php heading.cpp 

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/umbrello/headings/
	$(INSTALL_DATA) $(srcdir)/heading.java $(kde_datadir)/umbrello/headings/heading.java
	$(mkinstalldirs) $(kde_datadir)/umbrello/headings/
	$(INSTALL_DATA) $(srcdir)/heading.h $(kde_datadir)/umbrello/headings/heading.h
	$(mkinstalldirs) $(kde_datadir)/umbrello/headings/
	$(INSTALL_DATA) $(srcdir)/heading.php $(kde_datadir)/umbrello/headings/heading.php
	$(mkinstalldirs) $(kde_datadir)/umbrello/headings/
	$(INSTALL_DATA) $(srcdir)/heading.cpp $(kde_datadir)/umbrello/headings/heading.cpp

uninstall-local:
	-rm -f $(kde_datadir)/umbrello/headings/heading.java
	-rm -f $(kde_datadir)/umbrello/headings/heading.h
	-rm -f $(kde_datadir)/umbrello/headings/heading.php
	-rm -f $(kde_datadir)/umbrello/headings/heading.cpp

####### kdevelop will overwrite this part!!! (end)############
