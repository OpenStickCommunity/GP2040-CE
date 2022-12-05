const path = require('path');
const fs = require('fs');

const exec = require('child_process').execFile;

const root = path.dirname(require.main.filename).replace(path.normalize("www"), "");
const rootwww = path.dirname(require.main.filename);

const fsdata_filename = path.normalize("/lib/httpd/fsdata.c");

function correctincludes() {
    fs.readFile(root + fsdata_filename, "utf8", function (error, data) {
        if (data) {
            let result = data.replace("#include \"lwip/apps/fs.h\"\r\n#include \"lwip/def.h\"", '#include "fsdata.h"');
        
            fs.writeFile(root + fsdata_filename, result, "utf8", function (error) {
                if (error) {
                    console.error(error);
                }
            });
        } else {
            if (error) {
                console.error(error);
            }
        }
    });
}

function makefsdata() {
      exec(path.normalize(`${root}/tools/makefsdata`), [path.normalize(`${rootwww}/build`), `-f:`+ path.normalize(`${root}/lib/httpd/fsdata.c`)], function(error, data) {
        if (error) {
            console.error(error);
        } else {
            correctincludes();
        }
    });  
}
makefsdata();