RIME_ROOT = $(CURDIR)

sharedir = $(DESTDIR)/usr/share
bindir = $(DESTDIR)/usr/bin

.PHONY: all thirdparty clean librime librime-static install-librime uninstall-librime release install uninstall debug install-debug uninstall-debug test

all: release

thirdparty:
	make -f Makefile.thirdparty

thirdparty/%:
	make -f Makefile.thirdparty $(@:thirdparty/%=%)

clean:
	rm -Rf build build-static debug-build

librime: release
install-librime: install
uninstall-librime: uninstall

librime-static:
	cmake . -Bbuild-static -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DBUILD_SHARED_LIBS=OFF
	cmake --build build-static

wasm:
	cmake . -Bbuild --debug-output \
	-DBOOST_INCLUDEDIR:FILEPATH=/home/ztl8702/chronotext-boost/dist/emscripten/include \
	-DBOOST_LIBRARYDIR:FILEPATH=/home/ztl8702/chronotext-boost/dist/emscripten/lib \
	-DBUILD_TEST=OFF -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DBUILD_SHARED_LIBS=OFF \
	-DCMAKE_TOOLCHAIN_FILE=/home/ztl8702/emsdk-portable/emscripten/1.37.35/cmake/Modules/Platform/Emscripten.cmake \
	-DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS} -s WASM=1 -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0 --memory-init-file 0"
	cmake --build build

release:
	cmake . -Bbuild -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
	cmake --build build

debug:
	cmake . -Bdebug-build -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Debug
	cmake --build debug-build

install:
	cmake --build build --target install

install-debug:
	cmake --build debug-build --target install

uninstall:
	cmake --build build --target uninstall

uninstall-debug:
	cmake --build debug-build --target uninstall

test: release
	(cd build/test; ./rime_test)

test-debug: debug
	(cd debug-build/test; ./rime_test)

