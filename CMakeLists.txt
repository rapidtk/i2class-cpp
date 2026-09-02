cmake_minimum_required(VERSION 3.16)
project(i2class LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The stable, portable core (see COMPILATION_STATUS.md) -- parsers/, legacy/, benchmarks/,
# and the header-only backend stubs (rfile400/rfileodbc/rfileado/rfilelpt) are excluded.
add_library(i2class SHARED
	src/RPGTypes.cpp
	src/xxcvt.cpp
	src/xxdtaa.cpp
	src/as400.cpp
	src/compat/stdlib.cpp
	src/file/rfile.cpp
)

target_include_directories(i2class PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/file
)

# rfileodbc.cpp needs a real ODBC SDK (always present on Windows via odbc32.lib; on Linux
# needs unixODBC installed). Skip it -- and anything that depends on it -- if unavailable,
# rather than failing the whole configure/build on platforms without ODBC.
find_package(ODBC)
if(ODBC_FOUND)
	target_sources(i2class PRIVATE src/file/rfileodbc.cpp)
	target_link_libraries(i2class PUBLIC ODBC::ODBC)
else()
	message(STATUS "ODBC not found -- rfileodbc.cpp and i2class-demo (test_file()) will not be built")
endif()

# Mirrors the .vcxproj's Debug/Compatibility/Release configurations for anything that
# still checks I2_RUNTIME_MODE. Uses CMake's own standard config names (Debug/Release)
# rather than a custom "Compatibility" config, so this works unmodified on both
# single-config generators (Makefiles/Ninja) and multi-config ones (Visual Studio);
# anything that isn't Debug or Release (including no config at all) falls back to
# I2_RUNTIME_MODE_COMPATIBILITY, matching the old default.
target_compile_definitions(i2class PRIVATE
	I2_RUNTIME_MODE_DEBUGGABILITY=0
	I2_RUNTIME_MODE_COMPATIBILITY=1
	I2_RUNTIME_MODE_PERFORMANCE=2
	$<IF:$<CONFIG:Debug>,I2_RUNTIME_MODE=I2_RUNTIME_MODE_DEBUGGABILITY,$<IF:$<CONFIG:Release>,I2_RUNTIME_MODE=I2_RUNTIME_MODE_PERFORMANCE,I2_RUNTIME_MODE=I2_RUNTIME_MODE_COMPATIBILITY>>>
)

# Fixed<>/DS<>/etc. are header-only templates, so each consumer .cpp compiles its own
# instantiation -- PUBLIC (not PRIVATE) so this reaches anything that links against
# i2class too, not just the library's own .cpp files.
target_compile_definitions(i2class PUBLIC
	$<$<CONFIG:Debug>:I2_RUNTIME_ENABLE_BOUNDS_CHECK=1>
)

if(MSVC)
	target_compile_definitions(i2class PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# I2CLASS_BUILDING_DLL only applies while compiling this target itself (selects
# __declspec(dllexport) in i2compat.h's I2CLASS_API macro); I2CLASS_USING_DLL is an
# INTERFACE definition, so it's automatically propagated to anything that links
# against i2class (selecting __declspec(dllimport)) without needing to repeat it on
# every consumer target. The original .vcxproj (which links everything statically,
# no DLL) never defines either, so I2CLASS_API stays a no-op there -- unchanged.
target_compile_definitions(i2class PRIVATE I2CLASS_BUILDING_DLL)
target_compile_definitions(i2class INTERFACE I2CLASS_USING_DLL)

# MSVC exports nothing from a DLL by default (unlike GCC); this generates the export
# table automatically so no source file needs __declspec(dllexport) added. No effect
# on non-Windows targets (a .so already exports everything by default).
set_target_properties(i2class PROPERTIES
	WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# unixODBC ships no CSV/text driver, so off Windows the CUSTMAST rows are read from a
# SQLite database generated here from the same CUSTMAST.csv -- the CSV stays the single
# source of truth. Needs the libsqliteodbc package (driver "SQLite3") plus the sqlite3 CLI.
set(I2CLASS_DB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(CUSTMAST_DB_AVAILABLE FALSE)
if(ODBC_FOUND AND NOT WIN32)
	find_program(SQLITE3_EXECUTABLE sqlite3)
	if(SQLITE3_EXECUTABLE)
		set(I2CLASS_DB_DIR ${CMAKE_CURRENT_BINARY_DIR})
		set(CUSTMAST_DB ${CMAKE_CURRENT_BINARY_DIR}/CUSTMAST.db)
		# The SQL lives in a generated script rather than on the command line: CMake's
		# makefile escaping backslash-escapes spaces but leaves ( ) ; alone, so passing
		# "CREATE TABLE CUSTMAST(...)" as an argument is a /bin/sh syntax error.
		set(CUSTMAST_SQL ${CMAKE_CURRENT_BINARY_DIR}/custmast_import.sql)
		file(WRITE ${CUSTMAST_SQL}
"CREATE TABLE CUSTMAST(CUSNO INTEGER, CNAME TEXT, ORDVAL REAL);
.mode csv
.import --skip 1 ${CMAKE_CURRENT_SOURCE_DIR}/tests/CUSTMAST.csv CUSTMAST
")
		add_custom_command(
			OUTPUT ${CUSTMAST_DB}
			COMMAND ${CMAKE_COMMAND} -E rm -f ${CUSTMAST_DB}
			COMMAND ${SQLITE3_EXECUTABLE} ${CUSTMAST_DB} ".read ${CUSTMAST_SQL}"
			DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CUSTMAST.csv
			COMMENT "Generating CUSTMAST.db from CUSTMAST.csv"
		)
		add_custom_target(custmast_db ALL DEPENDS ${CUSTMAST_DB})
		set(CUSTMAST_DB_AVAILABLE TRUE)
	else()
		message(STATUS "sqlite3 CLI not found -- CUSTMAST.db cannot be generated, ODBC file tests will be built but not run")
	endif()
endif()

# Demo program (see i2class-cpp.cpp) -- links against the shared library like any
# other consumer would. test_file() needs the ODBC-backed RfileODBC, so this (and its
# CTest entry) only exist when ODBC was found above.
if(ODBC_FOUND)
	add_executable(i2class-demo i2class-cpp.cpp)
	target_link_libraries(i2class-demo PRIVATE i2class)
	# Absolute path to tests/CUSTMAST.csv's folder, for test_file()'s ODBC connection string.
	target_compile_definitions(i2class-demo PRIVATE
		I2CLASS_TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests"
		I2CLASS_DB_DIR="${I2CLASS_DB_DIR}")
	if(CUSTMAST_DB_AVAILABLE)
		add_dependencies(i2class-demo custmast_db)
	endif()
endif()

enable_testing()

add_executable(smoke_core tests/smoke_core.cpp)
target_link_libraries(smoke_core PRIVATE i2class)
add_test(NAME smoke_core COMMAND smoke_core)

add_executable(rfile_print_check tests/rfile_print_check.cpp)
target_link_libraries(rfile_print_check PRIVATE i2class)
add_test(NAME rfile_print_check COMMAND rfile_print_check)

add_executable(bounds_check tests/bounds_check.cpp)
target_link_libraries(bounds_check PRIVATE i2class)
add_test(NAME bounds_check COMMAND bounds_check)

add_executable(negative_zoned_check tests/negative_zoned_check.cpp)
target_link_libraries(negative_zoned_check PRIVATE i2class)
add_test(NAME negative_zoned_check COMMAND negative_zoned_check)

# RPG opcodes and BIFs that previously had no coverage (SCAN/CHECK/CAT/MOVEA/TESTN/
# XFOOT/SORTA/LOOKUP and the numeric increment operators).
add_executable(rpg_bif_check tests/rpg_bif_check.cpp)
target_link_libraries(rpg_bif_check PRIVATE i2class)
add_test(NAME rpg_bif_check COMMAND rpg_bif_check)

# Forces and exercises i2compat.h's portable (non-MSVC) _itoa_s/_ecvt_s fallback branch
# directly -- see the file's own header comment for why it #includes stdlib.cpp as source
# instead of linking against the i2class library.
add_executable(i2compat_fallback_check tests/i2compat_fallback_check.cpp)
target_include_directories(i2compat_fallback_check PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME i2compat_fallback_check COMMAND i2compat_fallback_check)

if(ODBC_FOUND)
	add_executable(rfileodbc_custmast_check tests/rfileodbc_custmast_check.cpp)
	target_link_libraries(rfileodbc_custmast_check PRIVATE i2class)
	target_compile_definitions(rfileodbc_custmast_check PRIVATE
		I2CLASS_TESTS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests"
		I2CLASS_DB_DIR="${I2CLASS_DB_DIR}")
	# Runs against the Windows-builtin Text driver, or the generated SQLite database
	# elsewhere; built either way so RfileODBC keeps getting compile coverage.
	if(WIN32 OR CUSTMAST_DB_AVAILABLE)
		add_test(NAME rfileodbc_custmast_check COMMAND rfileodbc_custmast_check)
	else()
		message(STATUS "rfileodbc_custmast_check will be built but not run -- no usable CSV/SQLite ODBC driver")
	endif()
	if(CUSTMAST_DB_AVAILABLE)
		add_dependencies(rfileodbc_custmast_check custmast_db)
	endif()
endif()
