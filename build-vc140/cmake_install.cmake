# Install script for directory: C:/Users/steph/Desktop/singularity/singularity/indra

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Singularity")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/cmake/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/aistatemachine/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llaudio/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llappearance/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llcharacter/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llcommon/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llimage/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/libopenjpeg/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/libhacd/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/libndhacd/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/libpathing/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llimagej2coj/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llinventory/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llmath/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llmessage/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llprimitive/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llrender/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llvfs/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llwindow/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llxml/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/copy_win_scripts/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llplugin/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/llui/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/plugins/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/newview/statemachine/cmake_install.cmake")
  include("C:/Users/steph/Desktop/singularity/singularity/build-vc140/newview/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/steph/Desktop/singularity/singularity/build-vc140/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
