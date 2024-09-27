# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "I:/Users/qqss6/Documents/GitHub/GP2040-CE/build-rp2040/_deps/arduinojson-src"
  "I:/Users/qqss6/Documents/GitHub/GP2040-CE/build-rp2040/_deps/arduinojson-build"
  "I:/Users/qqss6/Documents/GitHub/GP2040-CE/build-rp2040/_deps/arduinojson-subbuild/arduinojson-populate-prefix"
  "I:/Users/qqss6/Documents/GitHub/GP2040-CE/build-rp2040/_deps/arduinojson-subbuild/arduinojson-populate-prefix/tmp"
  "I:/Users/qqss6/Documents/GitHub/GP2040-CE/build-rp2040/_deps/arduinojson-subbuild/arduinojson-populate-prefix/src/arduinojson-populate-stamp"
  "I:/Users/qqss6/Documents/GitHub/GP2040-CE/build-rp2040/_deps/arduinojson-subbuild/arduinojson-populate-prefix/src"
  "I:/Users/qqss6/Documents/GitHub/GP2040-CE/build-rp2040/_deps/arduinojson-subbuild/arduinojson-populate-prefix/src/arduinojson-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "I:/Users/qqss6/Documents/GitHub/GP2040-CE/build-rp2040/_deps/arduinojson-subbuild/arduinojson-populate-prefix/src/arduinojson-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "I:/Users/qqss6/Documents/GitHub/GP2040-CE/build-rp2040/_deps/arduinojson-subbuild/arduinojson-populate-prefix/src/arduinojson-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
