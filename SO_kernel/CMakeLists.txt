
project(soForKernel LANGUAGES C)

add_library(${PROJECT_NAME} MODULE
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

target_link_libraries(${PROJECT_NAME}
  SceSysmemForDriver_stub
  SceThreadmgrForDriver_stub
  SceSblACMgrForDriver_stub
  SceSysclibForDriver_stub
)

VDSuiteCreateStubs(stubs soForKernel.emd
  LIBRARIES
    soForKernel_stub
)

install(TARGETS ${PROJECT_NAME}
  DESTINATION ${VDSUITE_LIBRARY_DIRECTORIES}
)

VDSuiteSignElf(${PROJECT_NAME}.skprx ${PROJECT_NAME} COMPRESS ON STRIP ON)
