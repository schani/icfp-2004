PERL = /usr/bin/perl            # full path required

BINDIR   = /usr/local/bin
#BINDIR  = $(HOME)/bin
MKBINDIR = true
BINMODE  = 755

all: perl

perl: perl.align perl.width

perl.align:
	$(PERL) -pi -e 's,^#!\s*/\S+/perl\S*(\s|$$),#!$(PERL)$$1,' align

perl.width:
	$(PERL) -pi -e 's,^#!\s*/\S+/perl\S*(\s|$$),#!$(PERL)$$1,' width

install: install.align install.width

install.align: perl.align
	-$(MKBINDIR) && mkdir $(BINDIR) 2> /dev/null
	cp align $(BINDIR)/.
	chmod $(BINMODE) $(BINDIR)/align

install.width: perl.width
	-$(MKBINDIR) && mkdir $(BINDIR) 2> /dev/null
	cp width $(BINDIR)/.
	chmod $(BINMODE) $(BINDIR)/width

deinstall: deinstall.align deinstall.width

deinstall.align:
	rm -f $(BINDIR)/align
	-$(MKBINDIR) && rmdir $(BINDIR) 2> /dev/null

deinstall.width:
	rm -f $(BINDIR)/width
	-$(MKBINDIR) && rmdir $(BINDIR) 2> /dev/null

clean:

# -----------------------------------------------------------------------
# for maintainer use only

import:
	cp $(HOME)/binp/align $(HOME)/binp/width .
	make perl

dist:
	VER=`./align -h 2>&1 | sed -n '$$s/.*ersion *//p'`; \
	BASE=align-$$VER; \
	rm -f ../$$BASE.tar.gz; \
	cp -r ../src ../$$BASE; \
	z -gz -s .tar.gz -m 644 ../$$BASE
