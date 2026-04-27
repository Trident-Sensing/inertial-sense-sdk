CMAKE_MINIMUM_REQUIRED(VERSION 3.10.0)

project(InertialSenseSDK)

file(GLOB SOURCES_SDK
    "src/*.c"
    "src/*.cpp"
    "src/*.h"
    "src/protocol/*.cpp"
    "src/protocol/*.h"
    "src/util/*.cpp"
    "src/util/*.h"
    "hw-libs/bootloader/bootloaderShared.c"
    "tests/runtime/*.cpp"
    "tests/runtime/*.h"
    "src/yaml-cpp/*.cpp"
    "src/yaml-cpp/*.h"
    "src/yaml-cpp/contrib/*.cpp"
    "src/yaml-cpp/contrib/*.h"
    "src/yaml-cpp/node/*.cpp"
    "src/yaml-cpp/node/*.h"
)

# Compile libusb from source
if (WIN32)
    file(GLOB SOURCES_LIB_USB
        "src/libusb/libusb/*.h"
        "src/libusb/libusb/*.c"
        "src/libusb/libusb/os/*windows*"
    )
else ()
    file(GLOB SOURCES_LIB_USB
        "src/libusb/libusb/*.h"
        "src/libusb/libusb/*.c"
        "src/libusb/libusb/os/*linux*"
        "src/libusb/libusb/os/*posix*"
    )
endif ()

# Ignore bootloader files
# list(FILTER SOURCES_SDK EXCLUDE REGEX "ISBootloader.*")
list(FILTER SOURCES_SDK EXCLUDE REGEX "ISBootloaderSony.*")
list(FILTER SOURCES_SDK EXCLUDE REGEX "ISBootloaderSTM32.*")

list(APPEND SOURCES_SDK ${SOURCES_LIB_USB})

# find_package(udev)
find_library(UDEV_LIB udev)

add_library(${PROJECT_NAME} STATIC ${SOURCES_SDK})
# Expose a stable, namespaced alias so downstream code can write
# target_link_libraries(... InertialSense::SDK) whether the SDK is consumed
# via add_subdirectory (in-tree) or via find_package (installed).
set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME SDK)
add_library(InertialSense::SDK ALIAS ${PROJECT_NAME})

# Include paths. BUILD_INTERFACE paths resolve during an in-tree build
# (e.g. via add_subdirectory); INSTALL_INTERFACE paths resolve for downstream
# consumers that did find_package(InertialSenseSDK) after this package was
# installed.
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/util>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/protocol>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/libusb>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/libusb/libusb>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/yaml-cpp>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tests/runtime>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external>
    $<INSTALL_INTERFACE:include/inertial_sense_sdk/src>
    $<INSTALL_INTERFACE:include/inertial_sense_sdk/src/util>
    $<INSTALL_INTERFACE:include/inertial_sense_sdk/src/protocol>
    $<INSTALL_INTERFACE:include/inertial_sense_sdk/src/libusb>
    $<INSTALL_INTERFACE:include/inertial_sense_sdk/src/libusb/libusb>
    $<INSTALL_INTERFACE:include/inertial_sense_sdk/src/yaml-cpp>
    $<INSTALL_INTERFACE:include/inertial_sense_sdk/tests/runtime>
)

# Build using C11 and CXX20
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

# Other settings
if (WIN32)
    # yamlcpp needs to know it's a static lib.
    add_definitions(-DYAML_CPP_STATIC_DEFINE)

    # Windows specific include dir
    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/libusb/msvc
    )

    # We need to ensure windows static libs are found
    target_link_libraries(${PROJECT_NAME} Ws2_32.lib)
else ()
    # Linux specific include dir
    target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src/libusb/linux>
        $<INSTALL_INTERFACE:include/inertial_sense_sdk/src/libusb/linux>
    )

    set_source_files_properties(
        src/libusb/libusb/core.c
        PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-variable"
    )

    # Linux compiler flags
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers")

    # Set Linux compiler linker flag
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

    # Link in Linux specific packages
    target_link_libraries(${PROJECT_NAME} udev m)
endif ()

# ---------------------------------------------------------------------------
# Install + export as a CMake package so downstream projects can do
#   find_package(InertialSenseSDK REQUIRED)
#   target_link_libraries(mytarget PRIVATE InertialSense::SDK)
# without touching source-tree relative paths.
# ---------------------------------------------------------------------------
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS ${PROJECT_NAME}
    EXPORT InertialSenseSDKTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Mirror the source tree under include/inertial_sense_sdk/ so the bare
# includes the SDK itself uses (#include "InertialSense.h", "util/foo.h",
# "libusb.h", ...) continue to resolve from INSTALL_INTERFACE include paths.
install(DIRECTORY src/
    DESTINATION include/inertial_sense_sdk/src
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "libusb/android" EXCLUDE
        PATTERN "libusb/doc" EXCLUDE
        PATTERN "libusb/examples" EXCLUDE
        PATTERN "libusb/msvc" EXCLUDE
        PATTERN "libusb/tests" EXCLUDE
        PATTERN "libusb/Xcode" EXCLUDE
        PATTERN "yaml-cpp/test" EXCLUDE
)
install(DIRECTORY tests/runtime/
    DESTINATION include/inertial_sense_sdk/tests/runtime
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

install(EXPORT InertialSenseSDKTargets
    FILE InertialSenseSDKTargets.cmake
    NAMESPACE InertialSense::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/InertialSenseSDK
)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/InertialSenseSDKConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/InertialSenseSDKConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/InertialSenseSDK
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/InertialSenseSDKConfigVersion.cmake
    VERSION 2.6.0
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/InertialSenseSDKConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/InertialSenseSDKConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/InertialSenseSDK
)
