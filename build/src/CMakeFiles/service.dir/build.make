# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/guagua/Desktop/work/EVM

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/guagua/Desktop/work/EVM/build

# Include any dependencies generated for this target.
include src/CMakeFiles/service.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include src/CMakeFiles/service.dir/compiler_depend.make

# Include the progress variables for this target.
include src/CMakeFiles/service.dir/progress.make

# Include the compile flags for this target's objects.
include src/CMakeFiles/service.dir/flags.make

src/CMakeFiles/service.dir/service.cc.o: src/CMakeFiles/service.dir/flags.make
src/CMakeFiles/service.dir/service.cc.o: ../src/service.cc
src/CMakeFiles/service.dir/service.cc.o: src/CMakeFiles/service.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/guagua/Desktop/work/EVM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/CMakeFiles/service.dir/service.cc.o"
	cd /home/guagua/Desktop/work/EVM/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/service.dir/service.cc.o -MF CMakeFiles/service.dir/service.cc.o.d -o CMakeFiles/service.dir/service.cc.o -c /home/guagua/Desktop/work/EVM/src/service.cc

src/CMakeFiles/service.dir/service.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/service.dir/service.cc.i"
	cd /home/guagua/Desktop/work/EVM/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/guagua/Desktop/work/EVM/src/service.cc > CMakeFiles/service.dir/service.cc.i

src/CMakeFiles/service.dir/service.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/service.dir/service.cc.s"
	cd /home/guagua/Desktop/work/EVM/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/guagua/Desktop/work/EVM/src/service.cc -o CMakeFiles/service.dir/service.cc.s

src/CMakeFiles/service.dir/device.cc.o: src/CMakeFiles/service.dir/flags.make
src/CMakeFiles/service.dir/device.cc.o: ../src/device.cc
src/CMakeFiles/service.dir/device.cc.o: src/CMakeFiles/service.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/guagua/Desktop/work/EVM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/CMakeFiles/service.dir/device.cc.o"
	cd /home/guagua/Desktop/work/EVM/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT src/CMakeFiles/service.dir/device.cc.o -MF CMakeFiles/service.dir/device.cc.o.d -o CMakeFiles/service.dir/device.cc.o -c /home/guagua/Desktop/work/EVM/src/device.cc

src/CMakeFiles/service.dir/device.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/service.dir/device.cc.i"
	cd /home/guagua/Desktop/work/EVM/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/guagua/Desktop/work/EVM/src/device.cc > CMakeFiles/service.dir/device.cc.i

src/CMakeFiles/service.dir/device.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/service.dir/device.cc.s"
	cd /home/guagua/Desktop/work/EVM/build/src && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/guagua/Desktop/work/EVM/src/device.cc -o CMakeFiles/service.dir/device.cc.s

# Object files for target service
service_OBJECTS = \
"CMakeFiles/service.dir/service.cc.o" \
"CMakeFiles/service.dir/device.cc.o"

# External object files for target service
service_EXTERNAL_OBJECTS =

src/libservice.a: src/CMakeFiles/service.dir/service.cc.o
src/libservice.a: src/CMakeFiles/service.dir/device.cc.o
src/libservice.a: src/CMakeFiles/service.dir/build.make
src/libservice.a: src/CMakeFiles/service.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/guagua/Desktop/work/EVM/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libservice.a"
	cd /home/guagua/Desktop/work/EVM/build/src && $(CMAKE_COMMAND) -P CMakeFiles/service.dir/cmake_clean_target.cmake
	cd /home/guagua/Desktop/work/EVM/build/src && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/service.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/CMakeFiles/service.dir/build: src/libservice.a
.PHONY : src/CMakeFiles/service.dir/build

src/CMakeFiles/service.dir/clean:
	cd /home/guagua/Desktop/work/EVM/build/src && $(CMAKE_COMMAND) -P CMakeFiles/service.dir/cmake_clean.cmake
.PHONY : src/CMakeFiles/service.dir/clean

src/CMakeFiles/service.dir/depend:
	cd /home/guagua/Desktop/work/EVM/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/guagua/Desktop/work/EVM /home/guagua/Desktop/work/EVM/src /home/guagua/Desktop/work/EVM/build /home/guagua/Desktop/work/EVM/build/src /home/guagua/Desktop/work/EVM/build/src/CMakeFiles/service.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/CMakeFiles/service.dir/depend

