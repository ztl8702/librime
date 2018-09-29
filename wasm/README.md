# Building librime for WebAssembly

The easiest way to build librime for WebAssembly is to use Docker. 

> Although in principle librime can be built on macOS or Windows, such will require modifications to the source code of both librime and thirdparty libraries.
>
> The reason behind this is that Emscripten is almost identical to Linux environment, so that most C/C++ libraries' location are the same. If building on macOS for example, `ifdef __EMSCRIPTEN__` needs to be added alongside `ifdef OS_MACOSX`.