
project(soForVsh LANGUAGES C)

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
	add_custom_command(OUTPUT "${CMAKE_BINARY_DIR}/${gxp_we}_gxp.o"
		COMMAND $ENV{SCE_PSP2_SDK_DIR}/host_tools/build/bin/psp2bin "${gxp}"
      -b2e PSP2,_binary_${gxp_we}_gxp_start,_binary_${gxp_we}_gxp_size,4 -o "${CMAKE_BINARY_DIR}/${gxp_we}_gxp.o"
		WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
		DEPENDS ${gxp}
		COMMENT "Building GXP object ${gxp_we}_gxp.o"
	)
	list(APPEND SHADER_OBJECTS "${CMAKE_BINARY_DIR}/${gxp_we}_gxp.o")
endforeach()

add_library(${PROJECT_NAME} MODULE
  ${SHADER_OBJECTS}
  soForVsh.emd
  src/main.c
  src/hooks.c
  src/patches.c
  src/memUtil.c
)

include_directories(SYSTEM
  ${VDSUITE_USER_INCLUDE_DIRECTORIES}
  ${PSP2_SNC_STANDARD_INCLUDE_DIRECTORIES}
  include
  ../SO_include_common
)

target_link_directories(${PROJECT_NAME}
  PRIVATE
  ${VDSUITE_LIBRARY_DIRECTORIES}
)

target_link_libraries(${PROJECT_NAME}
  taihen_stub_weak
  taihenModuleUtils_stub_weak
  vita2d_sys_stub_weak
  SceSysmem_stub_weak
  SceLibKernel_stub_weak
  SceGxm_stub_weak
  SceGxmInternal_stub_weak
  SceDbg_stub_weak
  SceSharedFb_stub_weak
  SceThreadmgr_stub_weak
  SceModulemgr_stub_weak
  ${CMAKE_CURRENT_BINARY_DIR}/../SO_kernel/stubs/libsoForVsh_stub.a
)

VDSuiteSignElf(${PROJECT_NAME}.suprx ${PROJECT_NAME} COMPRESS ON STRIP ON)

set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY ADDITIONAL_CLEAN_FILES "${SHADER_GXPS}") # Clean target doesn't handle this automatically
