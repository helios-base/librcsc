
ACLOCAL_AMFLAGS=-I m4

SUBDIRS = rcsc example src

bin_SCRIPTS = librcsc-config librcscenv

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = librcsc.pc

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

doc:
	@DOXYGEN@

clean-doc:
	-rm -rf doc

distclean-local:
	-rm -rf doc

EXTRA_DIST = \
	bootstrap \
	librcsc.pc.in \
	librcsc.spec.in \
	librcsc-config.in \
	librcscenv.in \
	Doxyfile.in \
	NEWS.en

CLEANFILES = *~
