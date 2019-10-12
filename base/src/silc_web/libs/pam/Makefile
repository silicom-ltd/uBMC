include ../../build/config.mk
include ../../build/module.mk
include ../../build/gccconfig.mk

PAM_OBJ_CFLAGS = -Isrc/ \
	$(CPPFLAGS) $(CFLAGS) $(EXTRA_CFLAGS) $(WFLAGS)

PAM_LIB = -lpam -lsilc_mgmtd -lsilc -lpthread
PAM_OBJ = src/pam.o

%.o: %.c
	$(CC) $(PAM_OBJ_CFLAGS) $(LUA_CFLAGS) $(FPIC) -c -o $@ $<

compile: $(PAM_OBJ)
	$(LINK) $(SHLIB_FLAGS) -o src/pam.so $(PAM_OBJ) $(PAM_LIB)
	mkdir -p dist$(LUA_LIBRARYDIR)
	cp src/pam.so dist$(LUA_LIBRARYDIR)/pam.so

clean: luaclean
	rm -f $(PAM_OBJ) src/*.o src/*.so src/*.a src/*.dll
