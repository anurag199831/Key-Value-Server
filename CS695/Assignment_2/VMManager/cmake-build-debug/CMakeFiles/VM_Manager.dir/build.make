# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

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
CMAKE_COMMAND = /home/pranav/.softwares/clion-2020.1/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/pranav/.softwares/clion-2020.1/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/pranav/CLionProjects/Assignment_2/VMManager

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/VM_Manager.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/VM_Manager.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/VM_Manager.dir/flags.make

CMakeFiles/VM_Manager.dir/main.cpp.o: CMakeFiles/VM_Manager.dir/flags.make
CMakeFiles/VM_Manager.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/VM_Manager.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/VM_Manager.dir/main.cpp.o -c /home/pranav/CLionProjects/Assignment_2/VMManager/main.cpp

CMakeFiles/VM_Manager.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VM_Manager.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pranav/CLionProjects/Assignment_2/VMManager/main.cpp > CMakeFiles/VM_Manager.dir/main.cpp.i

CMakeFiles/VM_Manager.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VM_Manager.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pranav/CLionProjects/Assignment_2/VMManager/main.cpp -o CMakeFiles/VM_Manager.dir/main.cpp.s

CMakeFiles/VM_Manager.dir/Manager.cpp.o: CMakeFiles/VM_Manager.dir/flags.make
CMakeFiles/VM_Manager.dir/Manager.cpp.o: ../Manager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/VM_Manager.dir/Manager.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/VM_Manager.dir/Manager.cpp.o -c /home/pranav/CLionProjects/Assignment_2/VMManager/Manager.cpp

CMakeFiles/VM_Manager.dir/Manager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VM_Manager.dir/Manager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pranav/CLionProjects/Assignment_2/VMManager/Manager.cpp > CMakeFiles/VM_Manager.dir/Manager.cpp.i

CMakeFiles/VM_Manager.dir/Manager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VM_Manager.dir/Manager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pranav/CLionProjects/Assignment_2/VMManager/Manager.cpp -o CMakeFiles/VM_Manager.dir/Manager.cpp.s

CMakeFiles/VM_Manager.dir/VM.cpp.o: CMakeFiles/VM_Manager.dir/flags.make
CMakeFiles/VM_Manager.dir/VM.cpp.o: ../VM.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/VM_Manager.dir/VM.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/VM_Manager.dir/VM.cpp.o -c /home/pranav/CLionProjects/Assignment_2/VMManager/VM.cpp

CMakeFiles/VM_Manager.dir/VM.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VM_Manager.dir/VM.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pranav/CLionProjects/Assignment_2/VMManager/VM.cpp > CMakeFiles/VM_Manager.dir/VM.cpp.i

CMakeFiles/VM_Manager.dir/VM.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VM_Manager.dir/VM.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pranav/CLionProjects/Assignment_2/VMManager/VM.cpp -o CMakeFiles/VM_Manager.dir/VM.cpp.s

CMakeFiles/VM_Manager.dir/VmManager.cpp.o: CMakeFiles/VM_Manager.dir/flags.make
CMakeFiles/VM_Manager.dir/VmManager.cpp.o: ../VmManager.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/VM_Manager.dir/VmManager.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/VM_Manager.dir/VmManager.cpp.o -c /home/pranav/CLionProjects/Assignment_2/VMManager/VmManager.cpp

CMakeFiles/VM_Manager.dir/VmManager.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VM_Manager.dir/VmManager.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pranav/CLionProjects/Assignment_2/VMManager/VmManager.cpp > CMakeFiles/VM_Manager.dir/VmManager.cpp.i

CMakeFiles/VM_Manager.dir/VmManager.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VM_Manager.dir/VmManager.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pranav/CLionProjects/Assignment_2/VMManager/VmManager.cpp -o CMakeFiles/VM_Manager.dir/VmManager.cpp.s

CMakeFiles/VM_Manager.dir/Graph.cpp.o: CMakeFiles/VM_Manager.dir/flags.make
CMakeFiles/VM_Manager.dir/Graph.cpp.o: ../Graph.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/VM_Manager.dir/Graph.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/VM_Manager.dir/Graph.cpp.o -c /home/pranav/CLionProjects/Assignment_2/VMManager/Graph.cpp

CMakeFiles/VM_Manager.dir/Graph.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/VM_Manager.dir/Graph.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/pranav/CLionProjects/Assignment_2/VMManager/Graph.cpp > CMakeFiles/VM_Manager.dir/Graph.cpp.i

CMakeFiles/VM_Manager.dir/Graph.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/VM_Manager.dir/Graph.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/pranav/CLionProjects/Assignment_2/VMManager/Graph.cpp -o CMakeFiles/VM_Manager.dir/Graph.cpp.s

# Object files for target VM_Manager
VM_Manager_OBJECTS = \
"CMakeFiles/VM_Manager.dir/main.cpp.o" \
"CMakeFiles/VM_Manager.dir/Manager.cpp.o" \
"CMakeFiles/VM_Manager.dir/VM.cpp.o" \
"CMakeFiles/VM_Manager.dir/VmManager.cpp.o" \
"CMakeFiles/VM_Manager.dir/Graph.cpp.o"

# External object files for target VM_Manager
VM_Manager_EXTERNAL_OBJECTS =

VM_Manager: CMakeFiles/VM_Manager.dir/main.cpp.o
VM_Manager: CMakeFiles/VM_Manager.dir/Manager.cpp.o
VM_Manager: CMakeFiles/VM_Manager.dir/VM.cpp.o
VM_Manager: CMakeFiles/VM_Manager.dir/VmManager.cpp.o
VM_Manager: CMakeFiles/VM_Manager.dir/Graph.cpp.o
VM_Manager: CMakeFiles/VM_Manager.dir/build.make
VM_Manager: /usr/lib/x86_64-linux-gnu/libgtkmm-3.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libatkmm-1.6.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libgdkmm-3.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libgiomm-2.4.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libpangomm-1.4.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libglibmm-2.4.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libgtk-3.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libgdk-3.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libpangocairo-1.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libpango-1.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libharfbuzz.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libatk-1.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libcairo-gobject.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libgio-2.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libcairomm-1.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libcairo.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libsigc-2.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libgdk_pixbuf-2.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libgobject-2.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libglib-2.0.so
VM_Manager: /usr/lib/x86_64-linux-gnu/libvirt.so
VM_Manager: CMakeFiles/VM_Manager.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable VM_Manager"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/VM_Manager.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/VM_Manager.dir/build: VM_Manager

.PHONY : CMakeFiles/VM_Manager.dir/build

CMakeFiles/VM_Manager.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/VM_Manager.dir/cmake_clean.cmake
.PHONY : CMakeFiles/VM_Manager.dir/clean

CMakeFiles/VM_Manager.dir/depend:
	cd /home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/pranav/CLionProjects/Assignment_2/VMManager /home/pranav/CLionProjects/Assignment_2/VMManager /home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug /home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug /home/pranav/CLionProjects/Assignment_2/VMManager/cmake-build-debug/CMakeFiles/VM_Manager.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/VM_Manager.dir/depend

