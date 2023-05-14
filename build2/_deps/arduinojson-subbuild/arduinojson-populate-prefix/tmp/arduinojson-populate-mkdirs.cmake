# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/kanuk/clone/GP2040-CE/build2/_deps/arduinojson-src"
  "C:/Users/kanuk/clone/GP2040-CE/build2/_deps/arduinojson-build"
  "C:/Users/kanuk/clone/GP2040-CE/build2/_deps/arduinojson-subbuild/arduinojson-populate-prefix"
  "C:/Users/kanuk/clone/GP2040-CE/build2/_deps/arduinojson-subbuild/arduinojson-populate-prefix/tmp"
  "C:/Users/kanuk/clone/GP2040-CE/build2/_deps/arduinojson-subbuild/arduinojson-populate-prefix/src/arduinojson-populate-stamp"
  "C:/Users/kanuk/clone/GP2040-CE/build2/_deps/arduinojson-subbuild/arduinojson-populate-prefix/src"
  "C:/Users/kanuk/clone/GP2040-CE/build2/_deps/arduinojson-subbuild/arduinojson-populate-prefix/src/arduinojson-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/kanuk/clone/GP2040-CE/build2/_deps/arduinojson-subbuild/arduinojson-populate-prefix/src/arduinojson-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/kanuk/clone/GP2040-CE/build2/_deps/arduinojson-subbuild/arduinojson-populate-prefix/src/arduinojson-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
