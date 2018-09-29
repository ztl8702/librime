em++ -v --bind --preload-file ../data/minimal@/data \
     -o rime_console.html \
     -std=c++11 \
     -Wall \
     -O1 \
     -s WASM=1 -s ASSERTIONS=1 -s DISABLE_EXCEPTION_CATCHING=0 \
     -s ALLOW_MEMORY_GROWTH=1 \
     -I../src/ \
     -I/root/chronotext-boost/dist/emscripten/include \
     -I../thirdparty/include \
     rime_console.cc \
     -L../build/lib -lrime \
     -L../thirdparty/lib -lglog -lyaml-cpp -lleveldb -lmarisa -lopencc \
     -L/root/chronotext-boost/dist/emscripten/lib \
     -lboost_signals -lboost_filesystem -lboost_regex -lboost_system -lboost_locale