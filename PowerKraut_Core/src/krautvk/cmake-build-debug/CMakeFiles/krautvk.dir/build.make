# CMAKE generated file: DO NOT EDIT!
# Generated by "MinGW Makefiles" Generator, CMake Version 3.10

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

SHELL = cmd.exe

# The CMake executable.
CMAKE_COMMAND = "C:\Program Files\JetBrains\CLion 2018.1.2\bin\cmake\bin\cmake.exe"

# The command to remove a file.
RM = "C:\Program Files\JetBrains\CLion 2018.1.2\bin\cmake\bin\cmake.exe" -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk\cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/krautvk.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/krautvk.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/krautvk.dir/flags.make

CMakeFiles/krautvk.dir/library.c.obj: CMakeFiles/krautvk.dir/flags.make
CMakeFiles/krautvk.dir/library.c.obj: CMakeFiles/krautvk.dir/includes_C.rsp
CMakeFiles/krautvk.dir/library.c.obj: ../library.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/krautvk.dir/library.c.obj"
	C:\PROGRA~1\MINGW-~1\X86_64~1.0-P\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles\krautvk.dir\library.c.obj   -c C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk\library.c

CMakeFiles/krautvk.dir/library.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/krautvk.dir/library.c.i"
	C:\PROGRA~1\MINGW-~1\X86_64~1.0-P\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk\library.c > CMakeFiles\krautvk.dir\library.c.i

CMakeFiles/krautvk.dir/library.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/krautvk.dir/library.c.s"
	C:\PROGRA~1\MINGW-~1\X86_64~1.0-P\mingw64\bin\gcc.exe $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk\library.c -o CMakeFiles\krautvk.dir\library.c.s

CMakeFiles/krautvk.dir/library.c.obj.requires:

.PHONY : CMakeFiles/krautvk.dir/library.c.obj.requires

CMakeFiles/krautvk.dir/library.c.obj.provides: CMakeFiles/krautvk.dir/library.c.obj.requires
	$(MAKE) -f CMakeFiles\krautvk.dir\build.make CMakeFiles/krautvk.dir/library.c.obj.provides.build
.PHONY : CMakeFiles/krautvk.dir/library.c.obj.provides

CMakeFiles/krautvk.dir/library.c.obj.provides.build: CMakeFiles/krautvk.dir/library.c.obj


# Object files for target krautvk
krautvk_OBJECTS = \
"CMakeFiles/krautvk.dir/library.c.obj"

# External object files for target krautvk
krautvk_EXTERNAL_OBJECTS =

krautvk.dll: CMakeFiles/krautvk.dir/library.c.obj
krautvk.dll: CMakeFiles/krautvk.dir/build.make
krautvk.dll: ../lib/glfw3.dll
krautvk.dll: CMakeFiles/krautvk.dir/linklibs.rsp
krautvk.dll: CMakeFiles/krautvk.dir/objects1.rsp
krautvk.dll: CMakeFiles/krautvk.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk\cmake-build-debug\CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C shared library krautvk.dll"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles\krautvk.dir\link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/krautvk.dir/build: krautvk.dll

.PHONY : CMakeFiles/krautvk.dir/build

CMakeFiles/krautvk.dir/requires: CMakeFiles/krautvk.dir/library.c.obj.requires

.PHONY : CMakeFiles/krautvk.dir/requires

CMakeFiles/krautvk.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles\krautvk.dir\cmake_clean.cmake
.PHONY : CMakeFiles/krautvk.dir/clean

CMakeFiles/krautvk.dir/depend:
	$(CMAKE_COMMAND) -E cmake_depends "MinGW Makefiles" C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk\cmake-build-debug C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk\cmake-build-debug C:\PowerKraut\PowerKraut\PowerKraut_Core\src\krautvk\cmake-build-debug\CMakeFiles\krautvk.dir\DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/krautvk.dir/depend

