em++ -o rime_console.html \
     -std=c++11 \
     -Wall \
     -O3 \
     -s WASM=1 -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0 \
     -s ALLOW_MEMORY_GROWTH=1 \
     -I ../src/ \
     -I/home/ztl8702/chronotext-boost/dist/emscripten/include \
     -I ../thirdparty/include \
     -L/home/ztl8702/chronotext-boost/dist/emscripten/lib \
     -L/home/ztl8702/dev/librime/thirdparty/lib ../build/lib/librime.a \
     /home/ztl8702/chronotext-boost/dist/emscripten/lib/libboost_filesystem.a \
     /home/ztl8702/chronotext-boost/dist/emscripten/lib/libboost_regex.a \
     /home/ztl8702/chronotext-boost/dist/emscripten/lib/libboost_signals.a \
     /home/ztl8702/chronotext-boost/dist/emscripten/lib/libboost_system.a \
     /home/ztl8702/chronotext-boost/dist/emscripten/lib/libboost_locale.a \
     ../thirdparty/lib/libglog.a \
     ../thirdparty/lib/libyaml-cpp.a \
     ../thirdparty/lib/libleveldb.a \
     ../thirdparty/lib/libmarisa.a \
     ../thirdparty/lib/libopencc.a \
     rime_console.cc 