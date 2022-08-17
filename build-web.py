import re
import os
import os.path

website_dir = "www/build/"
fsdata_filename = "lib/httpd/fsdata.c"

print("Building React app")
os.chdir("www")
print("Running npm install")
os.system("npm i")
print("Running npm run build")
os.system("npm run build")
os.chdir("..")
print("Done")

print("Regenerating " + fsdata_filename)
makefsdata = os.path.normpath('./tools/makefsdata')
os.chmod(makefsdata, 0o775)
os.system(makefsdata + " " + website_dir + " -f:" + fsdata_filename)
print("Done")

print("Replace includes")
f = open(fsdata_filename, "r")
res = re.sub("#include .*\n.*lwip\/def\.h", "#include \"fsdata.h", f.read())
f.close()
f = open(fsdata_filename, "w")
f.write(res)
f.close()
print("Done")
