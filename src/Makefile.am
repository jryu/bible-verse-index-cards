%.pb.cc %.pb.h: %.proto
	$(PROTOC) --proto_path=$(srcdir) --cpp_out=$(builddir) $^

dist_noinst_DATA = config.proto
BUILT_SOURCES = config.pb.cc

noinst_PROGRAMS = renderer
renderer_SOURCES = main.cpp config.pb.cc config.pb.h
renderer_CPPFLAGS = $(CAIRO_CFLAGS)
renderer_LDADD = $(CAIRO_LIBS) $(GFLAGS_LIBS) $(PROTOBUF_LIBS)
