mergeInto(LibraryManager.library, {
    onReplOutput: function (ptr, colorPtr, isJson) {
        let message = UTF8ToString(ptr);
        if (isJson && shouldFormat()) {
            let obj = JSON.parse(message);
            message = JSON.stringify(obj, null, 4);
        }
        color = UTF8ToString(colorPtr);
        writeToRight(message, isJson);
    },
    onReplError: function (ptr) {
        let message = UTF8ToString(ptr);
        color = "darkred";
        writeToRight(message);
    },
    onReplReset: function () {
        let div = document.getElementById("right");
        div.innerHTML = "";
        console.log("Reset called");
    },
    onCompileDone: function (ptr) {
        let div = document.getElementById("right");
        div.innerHTML = "";
        let content = UTF8ToString(ptr);
        if (shouldFormat()) {
            let xhr = new XMLHttpRequest();
            xhr.open("POST", "https://format.temware.site");
            xhr.setRequestHeader("Accept", "text/plain");
            xhr.setRequestHeader("Content-Type", "text/plain");
            xhr.onreadystatechange = xhrReadyState;
            xhr.send(content);
        }
        else {
            writeLines(content);
        }

        function writeLines(lines) {
            lines.split("\n").forEach(function (line) {
                if (line.indexOf('\n') !== -1) {
                    writeLines(line);
                }
                else {
                    temp = line.trim();
                    if (temp.startsWith("//")) {
                        color = "green";
                    }
                    else {
                        color = "black";
                    }
                    writeToRight(line);
                }
            });
        }
        function xhrReadyState(evt) {
            if (evt.target.readyState === 4) {
                writeLines(evt.target.responseText);
            }
        }
    },
    onExampleAcquired: function (ptr) {
        let message = UTF8ToString(ptr);
        let txt = document.getElementById("userInput");
        txt.value = message;
        updateLineNumbers(txt);
    }
});