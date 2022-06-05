# TemLang

See [website](https://www.temware.site/TemLang) for more information.

## Sample

Try it out [here](https://www.temware.site/TemLang/repl.html).

## Building

Compilation instructions assume you are in the repository directory.

``` bash
cd <repository destination>
```


### Compiler / Read-Evaluate-Print-Loop (REPL)

```bash
clang -Iinclude src/main.c -lm
```

### Web Browser REPL

Requires [Emscripten](https://emscripten.org/).

```bash
emcc -Iinclude/ src/repl.c \
    -lm -s MODULARIZE=1 \
    -s EXPORTED_FUNCTIONS='["_malloc","_getVersionString", "_reset", "_initialize", "_replExecute", "_getExample"]' \
    -s EXPORTED_RUNTIME_METHODS='["UTF8ToString", "stringToUTF8", "intArrayFromString","ALLOC_NORMAL","allocate"]' \
    --js-library src/web/repl_lib.js \
    -s ALLOW_MEMORY_GROWTH=1 \
    --embed-file "examples"
```

## License
[See here.](LICENSE.md)