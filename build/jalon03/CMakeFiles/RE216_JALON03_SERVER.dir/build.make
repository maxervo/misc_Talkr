# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/guillaume/Documents/Telecom/RE216/school-chat/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/guillaume/Documents/Telecom/RE216/school-chat/build

# Include any dependencies generated for this target.
include jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/depend.make

# Include the progress variables for this target.
include jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/progress.make

# Include the compile flags for this target's objects.
include jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/flags.make

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/flags.make
jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o: /home/guillaume/Documents/Telecom/RE216/school-chat/src/jalon03/common.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/guillaume/Documents/Telecom/RE216/school-chat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o"
	cd /home/guillaume/Documents/Telecom/RE216/school-chat/build/jalon03 && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o   -c /home/guillaume/Documents/Telecom/RE216/school-chat/src/jalon03/common.c

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/RE216_JALON03_SERVER.dir/common.c.i"
	cd /home/guillaume/Documents/Telecom/RE216/school-chat/build/jalon03 && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/guillaume/Documents/Telecom/RE216/school-chat/src/jalon03/common.c > CMakeFiles/RE216_JALON03_SERVER.dir/common.c.i

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/RE216_JALON03_SERVER.dir/common.c.s"
	cd /home/guillaume/Documents/Telecom/RE216/school-chat/build/jalon03 && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/guillaume/Documents/Telecom/RE216/school-chat/src/jalon03/common.c -o CMakeFiles/RE216_JALON03_SERVER.dir/common.c.s

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o.requires:

.PHONY : jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o.requires

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o.provides: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o.requires
	$(MAKE) -f jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/build.make jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o.provides.build
.PHONY : jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o.provides

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o.provides.build: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o


jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/flags.make
jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o: /home/guillaume/Documents/Telecom/RE216/school-chat/src/jalon03/server.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/guillaume/Documents/Telecom/RE216/school-chat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o"
	cd /home/guillaume/Documents/Telecom/RE216/school-chat/build/jalon03 && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o   -c /home/guillaume/Documents/Telecom/RE216/school-chat/src/jalon03/server.c

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/RE216_JALON03_SERVER.dir/server.c.i"
	cd /home/guillaume/Documents/Telecom/RE216/school-chat/build/jalon03 && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/guillaume/Documents/Telecom/RE216/school-chat/src/jalon03/server.c > CMakeFiles/RE216_JALON03_SERVER.dir/server.c.i

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/RE216_JALON03_SERVER.dir/server.c.s"
	cd /home/guillaume/Documents/Telecom/RE216/school-chat/build/jalon03 && /usr/bin/cc  $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/guillaume/Documents/Telecom/RE216/school-chat/src/jalon03/server.c -o CMakeFiles/RE216_JALON03_SERVER.dir/server.c.s

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o.requires:

.PHONY : jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o.requires

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o.provides: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o.requires
	$(MAKE) -f jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/build.make jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o.provides.build
.PHONY : jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o.provides

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o.provides.build: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o


# Object files for target RE216_JALON03_SERVER
RE216_JALON03_SERVER_OBJECTS = \
"CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o" \
"CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o"

# External object files for target RE216_JALON03_SERVER
RE216_JALON03_SERVER_EXTERNAL_OBJECTS =

jalon03/RE216_JALON03_SERVER: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o
jalon03/RE216_JALON03_SERVER: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o
jalon03/RE216_JALON03_SERVER: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/build.make
jalon03/RE216_JALON03_SERVER: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/guillaume/Documents/Telecom/RE216/school-chat/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C executable RE216_JALON03_SERVER"
	cd /home/guillaume/Documents/Telecom/RE216/school-chat/build/jalon03 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/RE216_JALON03_SERVER.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/build: jalon03/RE216_JALON03_SERVER

.PHONY : jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/build

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/requires: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/common.c.o.requires
jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/requires: jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/server.c.o.requires

.PHONY : jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/requires

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/clean:
	cd /home/guillaume/Documents/Telecom/RE216/school-chat/build/jalon03 && $(CMAKE_COMMAND) -P CMakeFiles/RE216_JALON03_SERVER.dir/cmake_clean.cmake
.PHONY : jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/clean

jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/depend:
	cd /home/guillaume/Documents/Telecom/RE216/school-chat/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/guillaume/Documents/Telecom/RE216/school-chat/src /home/guillaume/Documents/Telecom/RE216/school-chat/src/jalon03 /home/guillaume/Documents/Telecom/RE216/school-chat/build /home/guillaume/Documents/Telecom/RE216/school-chat/build/jalon03 /home/guillaume/Documents/Telecom/RE216/school-chat/build/jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : jalon03/CMakeFiles/RE216_JALON03_SERVER.dir/depend

