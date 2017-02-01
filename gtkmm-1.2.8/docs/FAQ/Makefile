# fixme, integrate this w/ autoconf/automake (hvr)

all: FAQ/index.html

FAQ/index.html: FAQ.sgml
	sgmltools -b html FAQ.sgml
	chmod -R a+r,g+w FAQ

post-FAQ:  FAQ/index.html
	tar cf - FAQ |gzip -3 | ssh gtkmm.sourceforge.net "cd /home/groups/gtkmm/htdocs/docs; gunzip | tar xvf - "

clean:
	-rm -rf FAQ
