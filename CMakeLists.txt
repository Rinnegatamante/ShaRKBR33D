cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(ShaRKBR33D C CXX)
include("${VITASDK}/share/vita.cmake" REQUIRED)
set(VITA_APP_NAME "ShaRKBR33D")
set(VITA_TITLEID  "SHRKBR33D")
set(VITA_VERSION  "01.00")

execute_process(COMMAND git describe --abbrev=6 --dirty --always
	OUTPUT_VARIABLE GIT_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-optimize-sibling-calls -fdiagnostics-color=always -g -Wl,-q,--allow-multiple-definition -D_GNU_SOURCE -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-optimize-sibling-calls -fpermissive -g -fdiagnostics-color=always -Wno-write-strings -D_GNU_SOURCE -std=c++11")

add_executable(ShaRKBR33D
  source/dialogs.cpp
  source/main.cpp
  source/network.cpp
  source/promoter.cpp
  source/ioapi.c
  source/unzip.c
  source/sha1.c
)

add_dependencies(ShaRKBR33D ShaRKBR33D.skprx ShaRKBR33D.suprx)

target_link_libraries(ShaRKBR33D
  -Wl,--whole-archive pthread -Wl,--no-whole-archive
  m
  z
  stdc++
  vitaGL
  vitashark
  SceShaccCgExt
  mathneon
  curl
  ssl
  crypto
  taihen_stub
  kubridge_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceAudio_stub
  SceAudioIn_stub
  SceCtrl_stub
  SceCommonDialog_stub
  SceDisplay_stub
  SceKernelDmacMgr_stub
  SceGxm_stub
  SceNet_stub
  SceNetCtl_stub
  SceShaccCg_stub
  -Wl,--whole-archive SceSysmodule_stub -Wl,--no-whole-archive
  ScePower_stub
  SceTouch_stub
  SceVshBridge_stub
  ScePromoterUtil_stub
  SceMotion_stub
  SceIme_stub
  SceHid_stub
)

add_executable(ShaRKBR33D_kernel
  kernel/main.c
)

add_executable(ShaRKBR33D_user
  user/main.c
)

target_link_libraries(ShaRKBR33D_kernel
  k
  SceSysmemForDriver_stub
  SceIofilemgrForDriver_stub
  SceDebugForDriver_stub
  SceFios2KernelForDriver_stub
  SceProcEventForDriver_stub
  SceAppMgrForDriver_stub
  SceAppMgr_stub
  taihenForKernel_stub
  gcc
)

target_link_libraries(ShaRKBR33D_user
  taihen_stub
  SceLibKernel_stub_weak
  k
  gcc
  SceIofilemgr_stub
  SceDisplay_stub
  SceCtrl_stub
  ScePower_stub
  SceVideodec_stub
  SceLibc_stub
  SceAppMgr_stub
)

set_target_properties(ShaRKBR33D_kernel PROPERTIES LINK_FLAGS "-nostdlib")
set_target_properties(ShaRKBR33D_user PROPERTIES LINK_FLAGS "-nostdlib")
vita_create_self(eboot.bin ShaRKBR33D UNSAFE)
vita_create_self(vitadb.skprx ShaRKBR33D_kernel UNSAFE CONFIG ${CMAKE_SOURCE_DIR}/kernel/exports.yml)
vita_create_self(vitadb.suprx ShaRKBR33D_user UNSAFE CONFIG ${CMAKE_SOURCE_DIR}/user/exports.yml)
vita_create_vpk(ShaRKBR33D.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE ${CMAKE_SOURCE_DIR}/livearea/icon0.png sce_sys/icon0.png
       ${CMAKE_SOURCE_DIR}/livearea/default_gate.png sce_sys/livearea/contents/default_gate.png
       ${CMAKE_SOURCE_DIR}/livearea/bg0.png sce_sys/livearea/contents/bg0.png
       ${CMAKE_SOURCE_DIR}/livearea/template.xml sce_sys/livearea/contents/template.xml
       ${CMAKE_BINARY_DIR}/vitadb.skprx vitadb.skprx
       ${CMAKE_BINARY_DIR}/vitadb.suprx vitadb.suprx
)
