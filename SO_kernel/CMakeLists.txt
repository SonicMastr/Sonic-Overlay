
project(soForKernel LANGUAGES C)

add_library(${PROJECT_NAME} STATIC
  main.c
  soForKernel.emd
)

include_directories(
  ${VDSUITE_KERNEL_INCLUDE_DIRECTORIES}
  ${VDSUITE_STANDARD_INCLUDE_DIRECTORIES}
  ../SO_include_common
)

target_link_directories(${PROJECT_NAME}
  PRIVATE
  ${VDSUITE_LIBRARY_DIRECTORIES}
)

VDSuiteCreateStubs(stubs soForKernel.emd
  LIBRARIES
    soForKernel_stub
)

install(TARGETS ${PROJECT_NAME}
  DESTINATION ${VDSUITE_LIBRARY_DIRECTORIES}
)
