let fs = require('fs');

let fsdata_filename = "./lib/httpd/fsdata.c";

fs.readFile(fsdata_filename, "utf8", function (error, data) {
    if (data) {
        let result = data.replace("#include \"lwip/apps/fs.h\"\r\n#include \"lwip/def.h\"", '#include "fsdata.h"');
    
        fs.writeFile(fsdata_filename, result, "utf8", function (error) {
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