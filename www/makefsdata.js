import path from "path";
import fs from "fs";

import { execFile } from "child_process";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);

const root = path.dirname(__filename).replace(path.normalize("www"), "");
const rootwww = path.dirname(__filename);

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
      execFile(path.normalize(process.platform !== "darwin" ? `${root}/tools/makefsdata` : `${root}/tools/makefsdata.darwin`), [path.normalize(`${rootwww}/build`), '-defl:10', '-xc:png,json', `-f:`+ path.normalize(`${root}/lib/httpd/fsdata.c`)], function(error, data) {
        if (error) {
            console.error(error);
        } else {
            correctincludes();
        }
    });  
}
makefsdata();