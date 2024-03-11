# TODO whitespace?
# TODO need to run 'git submodule update --init' in pico-sdk?

cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(guitar-tuner)

pico_sdk_init()


# TODO other sources
add_executable(guitar-tuner guitar_tuner.c)
# TODO 
# - only link against needed libs
# - only link on pico_stdio_usb on debug build? or doesn't matter?
# Add pico SDK libs.
target_link_libraries(guitar-tuner pico_stdlib hardware_adc)
# Add internal (core) lib. 
target_link_libraries(guitar-tuner libcore)


# TODO call this pico SDK settings? (with a comment)

pico_enable_stdio_usb(guitar-tuner 1)
# Generate .uf2 file on build.
pico_add_extra_outputs(guitar-tuner)


# Libcore uses make and not cmake. Expose it to cmake and build 
# it by invoking its makefile.

add_custom_target(build_libcore
		  COMMAND make -C ${CMAKE_SOURCE_DIR} -f libcore.mk)

add_library(libcore STATIC IMPORTED)
target_link_directories(libcore INTERFACE ${CMAKE_SOURCE_DIR}/../core)
# Note if libcore used cmake instead of make then this could inherit its include directories
# instead of needing to duplicate them here.
# TODO can reuse the include directories from compiler_vars.mk somehow?
target_include_directories(libcore INTERFACE
			   ${CMAKE_SOURCE_DIR}/../include 
			   ${CMAKE_SOURCE_DIR}/../CMSIS-DSP/Include
			   ${CMAKE_SOURCE_DIR}/../CMSIS_6/CMSIS/Core/Include)
set_target_properties(libcore PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../core/libcore.a)
add_dependencies(libcore build_libcore)

# TODO note 'make clean' in build/ dir won't clean libcore. can't add a clean_libcore and make it
# a dependecy on the clean target, so either note this shortcoming in README/documentation somewhere
# on how to clean it manually with 'make -f libcore.mk clean', or make clean_libcore a dependecy 
# of build_libcore so it gets cleaned every time before build, or invoke -B option to make to 
# always make all libcore targets
