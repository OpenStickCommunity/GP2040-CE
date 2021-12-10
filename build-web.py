import os
import os.path

website_dir = "www/build/"
fsdata_filename = "lib/httpd/fsdata.c"

print("Building React app")
os.chdir("www")
if not os.path.isdir("node_modules"):
  print("Running npm install")
  os.system("npm i")
print("Running npm run build")
os.system("npm run build")
os.chdir("..")
print("Done")

print("Regenerating " + fsdata_filename)
dirname = os.path.dirname(__file__)
makefsdata = os.path.join(dirname, 'tools/makefsdata')
os.system(makefsdata + " " + website_dir + " -f:" + fsdata_filename)
print("Done")

print("Replace includes")
os.system('perl -i -p0e "s/#include.*?"lwip\/def.h"/#include ""fsdata.h"/s" ' + fsdata_filename)
print("Done")
