cmake_minimum_required(VERSION 3.16)
project(dwyu)

find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Found CLANG ${CLANG_PACKAGE_VERSION}")
message(STATUS "Using CLANGConfig.cmake in: ${CLANG_DIR}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(dwyu src/main.cpp)

target_link_directories(dwyu PRIVATE ${LLVM_LIBRARY_DIRS})
target_compile_definitions(dwyu PRIVATE ${LLVM_DEFINITIONS})

target_include_directories(dwyu
  PRIVATE
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
)

target_compile_features(dwyu
  PRIVATE
  "cxx_std_17"
)

target_link_libraries(dwyu
  PRIVATE
  clangIndex
  clangTooling
  clangFrontend
  clangDriver
  clangSerialization
  clangParse
  clangSema
  clangAnalysis
  clangASTMatchers
  clangAST
  clangEdit
  clangLex
  clangBasic  
)

llvm_map_components_to_libnames(llvm_libs x86asmparser support option profiledata FrontendOpenMP)
target_link_libraries(dwyu PRIVATE LLVM)

install(TARGETS dwyu DESTINATION bin)
install(PROGRAMS scripts/dwyu.sh DESTINATION bin)
