include ../../build/config.mk
include ../../build/module.mk
include ../../build/gccconfig.mk

MGMTDCLIENT_OBJ_CFLAGS = -Isrc/ \
	$(CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(WFLAGS)

MGMTDCLIENT_LIB = -lsilc_mgmtd -lsilc -lpthread
MGMTDCLIENT_OBJ = src/mgmtdclient.o

%.o: %.c
	$(CC) $(MGMTDCLIENT_OBJ_CFLAGS) $(LUA_CFLAGS) $(FPIC) -g -c -o $@ $<

compile: $(MGMTDCLIENT_OBJ)
	$(LINK) $(SHLIB_FLAGS) -o src/mgmtdclient.so $(MGMTDCLIENT_OBJ) $(MGMTDCLIENT_LIB)
	mkdir -p dist$(LUA_LIBRARYDIR)
	cp src/mgmtdclient.so dist$(LUA_LIBRARYDIR)/mgmtdclient.so

clean: luaclean
	rm -f $(MGMTDCLIENT_OBJ) src/*.o src/*.so src/*.a src/*.dll dist$(LUA_LIBRARYDIR)/mgmtdclient.so
