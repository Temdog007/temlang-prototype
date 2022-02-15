# TemLang

See [website](https://www.temware.site/TemLang) for more information.

## Sample

Try it out [here](https://www.temware.site/TemLang).

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
    -o build/repl.js -lm -s MODULARIZE=1 \
    -s EXPORTED_FUNCTIONS='["_malloc","_getVersionString", "_reset", "_initialize", "_replExecute", "_getExample"]' \
    -s EXPORTED_RUNTIME_METHODS='["UTF8ToString", "stringToUTF8", "intArrayFromString","ALLOC_NORMAL","allocate"]' \
    --js-library src/web/repl_lib.js \
    -s ALLOW_MEMORY_GROWTH=1 \
    --embed-file "examples"
```

## License
 The MIT License (MIT)

Copyright © 2022 <copyright holders>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

