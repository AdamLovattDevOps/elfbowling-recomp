# 3x hi-res art (port/src/hires.c, docs/HIRES.md). Included by port/Makefile.
# hires.o goes into libshim.a next to gdi.o; the matched units that carry the
# `#ifdef PORT` hooks include src/hires.h.
HIRES_HDRS := src/hires.h src/third_party/stb_image.h
$(B)/libshim.a: $(B)/hires.o
$(B)/hires.o $(B)/gdi.o: $(HIRES_HDRS)
$(NATIVE_OBJS): src/hires.h
