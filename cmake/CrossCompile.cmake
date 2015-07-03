SET(XCOMPILE_BASE_PATH ""                                 CACHE STRING "Path to the compiler")
SET(XCOMPILE_PREFIX    "arm-bcm2708hardfp-linux-gnueabi-" CACHE STRING "Toolchain prefix")
SET(XCOMPILE_LIB_PATH ""                                  CACHE STRING "Path to the libraries")

SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER ${XCOMPILE_BASE_PATH}/bin/${XCOMPILE_PREFIX}gcc)

SET(CMAKE_CXX_COMPILER ${XCOMPILE_BASE_PATH}/bin/${XCOMPILE_PREFIX}g++)

# where is the target environment
SET(CMAKE_FIND_ROOT_PATH ${XCOMPILE_LIB_PATH})

