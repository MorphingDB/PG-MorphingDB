subdir = src/model
top_builddir = ../..
include $(top_builddir)/src/Makefile.global


install: installdirs
	$(INSTALL_SCRIPT)  spiece.model '$(DESTDIR)$(libdir)/model/spiece.model'
	$(INSTALL_SCRIPT)  defect.pt '$(DESTDIR)$(libdir)/model/defect.pt'
	$(INSTALL_SCRIPT)  sst2.pt '$(DESTDIR)$(libdir)/model/sst2.pt'

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(libdir)/model'