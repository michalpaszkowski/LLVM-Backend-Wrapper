cmake_minimum_required(VERSION 3.13.4)
project(backend-wrapper)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(CMAKE_CXX_STANDARD 14)
include(GNUInstallDirs)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_library(backend-wrapper SHARED src/backend-wrapper.cpp)

llvm_map_components_to_libnames(
        llvm_libs 
        AllTargetsAsmParsers
        AllTargetsCodeGens
        AllTargetsDescs
        AllTargetsInfos
        Analysis
        AsmParser
        AsmPrinter
        CodeGen
        Core
        IRReader
        MC
        MIRParser
        Remarks
        ScalarOpts
        SelectionDAG
        Support
        Target
        TransformUtils
        Vectorize
        )

target_include_directories(
        ${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(backend-wrapper ${llvm_libs})

target_compile_definitions(
        ${PROJECT_NAME} PRIVATE -D_EXPORT
)
