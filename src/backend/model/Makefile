
subdir = src/backend/model
top_builddir = ../../..
include $(top_builddir)/src/Makefile.global
LIBTORCH_DIR = /home/postgres/libtorch
LIBTORCH_INCLUDES = -I$(LIBTORCH_DIR)/include -I$(LIBTORCH_DIR)/include/torch/csrc/api/include


override CPPFLAGS := -I. $(CPPFLAGS) $(LIBTORCH_INCLUDES)

OBJS = libtorch_wrapper.o


all: all-lib

# Shared library stuff
include $(top_srcdir)/src/Makefile.shlib

install: all installdirs
	$(INSTALL_STLIB) $(shlib) '$(DESTDIR)$(pkglibdir)/$(shlib)'

installdirs:
	$(MKDIR_P) '$(DESTDIR)$(pkglibdir)'

uninstall:
	rm -f '$(DESTDIR)$(pkglibdir)/$(shlib)'

clean distclean maintainer-clean:
	rm -f $(OBJS) $(shlib)








# torch_model_wrapper.o: torch_model_wrapper.cpp torch_model_wrapper.h
# 	$(COMPILE.cpp) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
