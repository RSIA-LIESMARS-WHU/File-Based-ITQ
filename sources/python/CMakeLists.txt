FIND_PACKAGE(PythonLibs 2.7 REQUIRED)
FIND_PACKAGE(Boost REQUIRED)

INCLUDE_DIRECTORIES(
    $ENV{Boost_DIR}
    ${PYTHON_INCLUDE_DIRS}
)

ADD_DEFINITIONS("-DLIBLINE_BUILD")

IF(WIN32)
    ADD_LIBRARY(
        pyitq
        SHARED
        boostpy_vc.cpp
        pyitq.cpp
    )
ELSE()
    ADD_LIBRARY(
        pyitq
        SHARED
        boostpy_ms.cpp
        pyitq.cpp
    )
ENDIF()

TARGET_LINK_LIBRARIES(
    pyitq
    ${PYTHON_LIBRARIES}
)

IF(WIN32)
    SET_TARGET_PROPERTIES(
        pyitq
        PROPERTIES
        SUFFIX
        ".pyd"
    )
ELSE()
    SET_TARGET_PROPERTIES(
        pyitq
        PROPERTIES
        SUFFIX
        ".so"
    )
ENDIF()

SET(LIBRARY_OUTPUT_PATH ${LSHBOX_BINARY_DIR}/py_module/${SAVE_CLASS})