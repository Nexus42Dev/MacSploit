# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

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

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CMake.app/Contents/bin/cmake

# The command to remove a file.
RM = /Applications/CMake.app/Contents/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build

# Include any dependencies generated for this target.
include test/CMakeFiles/enforce-checks-test.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include test/CMakeFiles/enforce-checks-test.dir/compiler_depend.make

# Include the progress variables for this target.
include test/CMakeFiles/enforce-checks-test.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/enforce-checks-test.dir/flags.make

test/CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.o: test/CMakeFiles/enforce-checks-test.dir/flags.make
test/CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.o: /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/test/enforce-checks-test.cc
test/CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.o: test/CMakeFiles/enforce-checks-test.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.o"
	cd /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT test/CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.o -MF CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.o.d -o CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.o -c /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/test/enforce-checks-test.cc

test/CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.i"
	cd /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/test/enforce-checks-test.cc > CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.i

test/CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.s"
	cd /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build/test && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/test/enforce-checks-test.cc -o CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.s

# Object files for target enforce-checks-test
enforce__checks__test_OBJECTS = \
"CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.o"

# External object files for target enforce-checks-test
enforce__checks__test_EXTERNAL_OBJECTS =

bin/enforce-checks-test: test/CMakeFiles/enforce-checks-test.dir/enforce-checks-test.cc.o
bin/enforce-checks-test: test/CMakeFiles/enforce-checks-test.dir/build.make
bin/enforce-checks-test: test/libtest-main.a
bin/enforce-checks-test: libfmt.a
bin/enforce-checks-test: test/gtest/libgtest.a
bin/enforce-checks-test: test/CMakeFiles/enforce-checks-test.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../bin/enforce-checks-test"
	cd /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/enforce-checks-test.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/enforce-checks-test.dir/build: bin/enforce-checks-test
.PHONY : test/CMakeFiles/enforce-checks-test.dir/build

test/CMakeFiles/enforce-checks-test.dir/clean:
	cd /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build/test && $(CMAKE_COMMAND) -P CMakeFiles/enforce-checks-test.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/enforce-checks-test.dir/clean

test/CMakeFiles/enforce-checks-test.dir/depend:
	cd /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/test /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build/test /Users/Nexus42/Documents/ProjectAbyssMac/Client/fmt/build/test/CMakeFiles/enforce-checks-test.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/enforce-checks-test.dir/depend

