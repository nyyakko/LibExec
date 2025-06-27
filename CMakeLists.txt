cmake_minimum_required(VERSION 3.25)

project(LibExec LANGUAGES CXX)

include(cmake/get_cpm.cmake)

CPMAddPackage("gh:nyyakko/LibError#master")

include(cmake/static_analyzers.cmake)
include(GNUInstallDirs)

set(LibExec_CompilerOptions ${LibExec_CompilerOptions} -Wno-gnu-statement-expression-from-macro-expansion)
# set(LibExec_LinkerOptions ${LibExec_LinkerOptions})

set(LibExec_ExternalLibraries
    LibError::LibError
)

add_subdirectory(LibExec)

