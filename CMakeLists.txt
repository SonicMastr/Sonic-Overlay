cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "$ENV{SCE_PSP2_SDK_DIR}/host_tools/build/cmake/psp2-snc-toolchain.cmake")

project(sonicoverlay LANGUAGES C)

include(VitaDevelopmentSuite)
enable_language(EMD)
set(CMAKE_C_STANDARD 99)

add_compile_options(
  -Xdiag=0 -Xquit=2
)

add_link_options(
  --prx-no-runtime-support
  --strip-unused-data
  --strip-duplicates
)

include_directories(SYSTEM
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
  ${VDSUITE_STANDARD_INCLUDE_DIRECTORIES}
  include
)

link_directories(
  ${VDSUITE_LIBRARY_DIRECTORIES}
)


set(VERTEX_SHADERS
  shaders/texture_v.cg
  shaders/color_v.cg
)

set(FRAGMENT_SHADERS
  shaders/texture_f.cg
  shaders/color_f.cg
)

foreach(shader ${VERTEX_SHADERS})
	get_filename_component(shader_we ${shader} NAME_WE)
	add_custom_command(OUTPUT "${shader_we}.gxp"
		COMMAND $ENV{SCE_PSP2_SDK_DIR}/host_tools/bin/psp2cgc -profile sce_vp_psp2 "${PROJECT_SOURCE_DIR}/${shader}"
			-o "${CMAKE_BINARY_DIR}/${shader_we}.gxp"
		DEPENDS ${shader}
		COMMENT "Compiling ${shader} to ${shader_we}.gxp"
	)
	list(APPEND SHADER_GXPS "${shader_we}.gxp")
endforeach()

foreach(shader ${FRAGMENT_SHADERS})
	get_filename_component(shader_we ${shader} NAME_WE)
	add_custom_command(OUTPUT "${shader_we}.gxp"
		COMMAND $ENV{SCE_PSP2_SDK_DIR}/host_tools/bin/psp2cgc -cache -profile sce_fp_psp2 "${PROJECT_SOURCE_DIR}/${shader}"
			-o "${CMAKE_BINARY_DIR}/${shader_we}.gxp"
		DEPENDS ${shader}
		COMMENT "Compiling ${shader} to ${shader_we}.gxp"
	)
	list(APPEND SHADER_GXPS "${shader_we}.gxp")
endforeach()

foreach(gxp ${SHADER_GXPS})
	get_filename_component(gxp_we ${gxp} NAME_WE)
	add_custom_command(OUTPUT "${gxp_we}_gxp.o"
		COMMAND $ENV{SCE_PSP2_SDK_DIR}/host_tools/build/bin/psp2bin "${gxp}"
      -b2e PSP2,_binary_${gxp_we}_gxp_start,_binary_${gxp_we}_gxp_size,4 -o "${CMAKE_BINARY_DIR}/${gxp_we}_gxp.o"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		DEPENDS ${gxp}
		COMMENT "Objcopying ${gxp} to ${gxp_we}_gxp.o"
	)
	list(APPEND SHADER_OBJECTS "${gxp_we}_gxp.o")
endforeach()

add_library(${PROJECT_NAME} MODULE
  ${SHADER_OBJECTS}
  sonicoverlay.emd
  src/main.c
  src/hooks.c
  src/patches.c
  src/memUtil.c
)

target_link_libraries(${PROJECT_NAME}
  taihen_stub_weak
  taihenModuleUtils_stub_weak
  SceGxmInternal_stub_weak
  SceGxm_stub_weak
  SceDbg_stub_weak
  SceSysmem_stub_weak
  SceLibKernel_stub_weak
  SceThreadmgr_stub_weak
)

VDSuiteSignElf(${PROJECT_NAME}.suprx ${PROJECT_NAME})