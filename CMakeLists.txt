cmake_minimum_required(VERSION 3.3.2)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(SDD_COMPILE_FLAGS
	-Wall
	-Wextra
	-Werror
	-pedantic
	-std=c11
	-Og
	-g
	-Dsdhmap_malloc=custom_malloc
	-Dsdhmap_realloc=custom_realloc
	-Dsdhmap_free=custom_free
	)

set(SDSTR_COMPILE_FLAGS ${SDD_COMPILE_FLAGS})
set(SDVEC_COMPILE_FLAGS ${SDD_COMPILE_FLAGS})
set(SDMAP_COMPILE_FLAGS ${SDD_COMPILE_FLAGS})
set(SDSET_COMPILE_FLAGS ${SDD_COMPILE_FLAGS})
set(SDHMAP_COMPILE_FLAGS ${SDD_COMPILE_FLAGS})
set(SDHSET_COMPILE_FLAGS ${SDD_COMPILE_FLAGS})

project(sdd)

#

add_library(sdmap src/sdmap.c)
target_include_directories(sdmap PUBLIC include)
target_compile_options(sdmap PUBLIC ${SDMAP_COMPILE_FLAGS})
install(TARGETS sdmap ARCHIVE DESTINATION lib)

add_library(sdhmap src/sdhmap.c)
target_include_directories(sdhmap PUBLIC include)
target_compile_options(sdhmap PUBLIC ${SDMAP_COMPILE_FLAGS})
install(TARGETS sdhmap ARCHIVE DESTINATION lib)

add_library(sdstr src/sdstr.c)
target_include_directories(sdstr PUBLIC include)
target_compile_options(sdstr PUBLIC ${SDST_COMPILE_FLAGS})
install(TARGETS sdstr ARCHIVE DESTINATION lib)

#

add_executable(tests_sdmap src/test_sdmap.c)
target_compile_options(tests_sdmap PUBLIC ${SDD_COMPILE_FLAGS})
target_link_libraries(tests_sdmap sdmap)

#

add_executable(tests_sdhmap src/test_sdhmap.c)
target_compile_options(tests_sdhmap PUBLIC ${SDD_COMPILE_FLAGS})
target_link_libraries(tests_sdhmap sdhmap)

#

add_executable(tests_sdstr src/test_sdstr.c)
target_compile_options(tests_sdstr PUBLIC ${SDD_COMPILE_FLAGS})
target_link_libraries(tests_sdstr sdstr)