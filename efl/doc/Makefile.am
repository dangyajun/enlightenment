MAINTAINERCLEANFILES = Makefile.in 

.PHONY: doc

PACKAGE_DOCNAME = $(PACKAGE_TARNAME)-$(PACKAGE_VERSION)-doc

if EFL_BUILD_DOC

doc-clean:
	rm -rf html/ latex/ man/ xml/ $(top_builddir)/$(PACKAGE_DOCNAME).tar*

doc: all
	$(efl_doxygen)
	cp $(srcdir)/img/* html/
	cp $(srcdir)/img/*.eps latex/
	rm -rf $(PACKAGE_DOCNAME).tar*
	mkdir -p $(PACKAGE_DOCNAME)/doc
	cp -R html/ latex/ man/ $(PACKAGE_DOCNAME)/doc
	tar cf $(PACKAGE_DOCNAME).tar $(PACKAGE_DOCNAME)/
	bzip2 -9 $(PACKAGE_DOCNAME).tar
	rm -rf $(PACKAGE_DOCNAME)/
	mv $(PACKAGE_DOCNAME).tar.bz2 $(top_builddir)
	@echo "Documentation Package: doc/$(PACKAGE_DOCNAME).tar.bz2"
	@echo "Documentation HTML: doc/html"

clean-local: doc-clean

else

doc:
	@echo "Documentation not built. Run ./configure --help"

endif

EXTRA_DIST = \
$(srcdir)/Doxyfile.in \
$(wildcard $(srcdir)/img/*.*) \
$(srcdir)/e.css \
$(srcdir)/head.html \
$(srcdir)/foot.html
