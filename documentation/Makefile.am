if HAVE_DOXYGEN
doxyfile.stamp:
	doxygen Doxyfile
	echo Timestamp > doxyfile.stamp

CLEANFILES = doxyfile.stamp

all-local: doxyfile.stamp
clean-local:
	rm -rf doxy
endif
