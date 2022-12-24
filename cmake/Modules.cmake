# Adds a C++20 module to the system.
#
# Based on the Clang documentation
# https://clang.llvm.org/docs/StandardCPlusPlusModules.html
#
# Note this only supports primary module interface units. The other types are
# less trivial. It's easier to wait for real module support in CMake before
# improving the modules.
function(add_module name)
	set(input ${CMAKE_CURRENT_SOURCE_DIR}/${name}.cppm)
	set(output ${PREBUILT_MODULE_PATH}/${name}.pcm)
	add_custom_command(
	    OUTPUT ${output}
		COMMAND
			${CMAKE_CXX_COMPILER}
			-std=c++2b
			-stdlib=libc++
			-fexperimental-library
			-fprebuilt-module-path=${PREBUILT_MODULE_PATH}
			--precompile
			${input}
			-o ${output}
		DEPENDS
			${input}
	)
	add_custom_target(${name}
		DEPENDS ${output}
	)
endfunction()
