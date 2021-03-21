cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "$ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cmake/psp2-snc-toolchain.cmake")

project(soForKernel)

include(VitaDevelopmentSuite)
enable_language(EMD)
set(CMAKE_C_STANDARD 99)

add_compile_options(
  -Xdiag=0 -Xquit=2 -O3
)

include_directories(
  ${VDSUITE_KERNEL_INCLUDE_DIRECTORIES}
  ../SO_include_common
)

link_directories(
  ${VDSUITE_LIBRARY_DIRECTORIES}
)

VDSuiteCreateStubs(stubs soForKernel.emd
  LIBRARIES
    soForKernel_stub
)

add_library(${PROJECT_NAME} STATIC
  main.c
  soForKernel.emd
)