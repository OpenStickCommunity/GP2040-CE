﻿# CMAKE generated file: DO NOT EDIT!
# Generated by "NMake Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\Users\kanuk\clone\GP2040-CE

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\Users\kanuk\clone\GP2040-CE\build2

# Utility rule file for ContinuousStart.

# Include any custom commands dependencies for this target.
include MPG\CMakeFiles\ContinuousStart.dir\compiler_depend.make

# Include the progress variables for this target.
include MPG\CMakeFiles\ContinuousStart.dir\progress.make

MPG\CMakeFiles\ContinuousStart:
	cd C:\Users\kanuk\clone\GP2040-CE\build2\MPG
	echo >nul && "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\ctest.exe" -D ContinuousStart
	cd C:\Users\kanuk\clone\GP2040-CE\build2

ContinuousStart: MPG\CMakeFiles\ContinuousStart
ContinuousStart: MPG\CMakeFiles\ContinuousStart.dir\build.make
.PHONY : ContinuousStart

# Rule to build all files generated by this target.
MPG\CMakeFiles\ContinuousStart.dir\build: ContinuousStart
.PHONY : MPG\CMakeFiles\ContinuousStart.dir\build

MPG\CMakeFiles\ContinuousStart.dir\clean:
	cd C:\Users\kanuk\clone\GP2040-CE\build2\MPG
	$(CMAKE_COMMAND) -P CMakeFiles\ContinuousStart.dir\cmake_clean.cmake
	cd C:\Users\kanuk\clone\GP2040-CE\build2
.PHONY : MPG\CMakeFiles\ContinuousStart.dir\clean

MPG\CMakeFiles\ContinuousStart.dir\depend:
	$(CMAKE_COMMAND) -E cmake_depends "NMake Makefiles" C:\Users\kanuk\clone\GP2040-CE C:\Users\kanuk\clone\GP2040-CE\MPG C:\Users\kanuk\clone\GP2040-CE\build2 C:\Users\kanuk\clone\GP2040-CE\build2\MPG C:\Users\kanuk\clone\GP2040-CE\build2\MPG\CMakeFiles\ContinuousStart.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : MPG\CMakeFiles\ContinuousStart.dir\depend

