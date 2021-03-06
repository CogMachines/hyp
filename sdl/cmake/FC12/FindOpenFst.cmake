# OPENFST_FOUND
# OPENFST_DIR

# add both these to -I and -DHAVE_OPENFST
# OPENFST_SOURCE_DIR = ${OPENFST_DIR}/src (library sources will be included with this added to -I)
# OPENFST_INCLUDE_DIR = ${OPENFST_SOURCE_DIR}/include
#IF(APPLE)
unset(OPENFST_INCLUDE_DIR CACHE)
sdl_find_path(OPENFST_INCLUDE_DIR fst.h ${OPENFST_ROOT}/src/include/fst)
#ELSE(APPLE)
#ENDIF(APPLE)

IF(OPENFST_INCLUDE_DIR)
 SET(OPENFST_FOUND 1)
 STRING(REGEX REPLACE "src/include/fst$" "" OPENFST_DIR ${OPENFST_INCLUDE_DIR})
 SET(OPENFST_SRC_DIR "${OPENFST_DIR}/src")
 SET(OPENFST_INCLUDE_DIR "${OPENFST_SRC_DIR}/include")
 MESSAGE(STATUS "Found OpenFst sources: ${OPENFST_SRC_DIR}")
ENDIF()
