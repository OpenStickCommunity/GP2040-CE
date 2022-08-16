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
makefsdata = '.\\tools\\makefsdata'
os.system(makefsdata + " " + website_dir + " -f:" + fsdata_filename)
print("Done")

print("Replace includes")
f = open(fsdata_filename, "rb")
res = re.sub("#include .*\n.*lwip\/def\.h", "#include \"fsdata.h", f.read())
f.close()
f = open(fsdata_filename, "wb")
f.write(res)
f.close()
print("Done")
