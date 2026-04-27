cmake_minimum_required(VERSION 3.10.0)
project(inertial_sense_ros2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(ament_cmake_gtest REQUIRED)
find_package(Threads REQUIRED)
find_package(yaml-cpp REQUIRED)

# Consume the Inertial Sense C++ SDK as a proper installed CMake package.
# Build it first via the top-level libs/inertial-sense-sdk/CMakeLists.txt and
# install it to a prefix on CMAKE_PREFIX_PATH. The imported target
# InertialSense::SDK carries its own include dirs + link deps transitively.
find_package(InertialSenseSDK REQUIRED)

add_definitions(-DROS2)
add_compile_definitions(PLATFORM_IS_LINUX)
add_compile_options(-fms-extensions)

rosidl_generate_interfaces(${PROJECT_NAME}
    "msg/GTime.msg"
    "msg/SatInfo.msg"
    "msg/GPS.msg"
    "msg/GPSInfo.msg"
    "msg/PIMU.msg"
    "msg/RTKInfo.msg"
    "msg/RTKRel.msg"
    "msg/GlonassEphemeris.msg"
    "msg/GNSSEphemeris.msg"
    "msg/GNSSObservation.msg"
    "msg/GNSSObsVec.msg"
    "msg/INL2States.msg"
    "msg/DIDINS2.msg"
    "msg/DIDINS1.msg"
    "msg/DIDINS4.msg"
    "srv/FirmwareUpdate.srv"
    "srv/RefLLAUpdate.srv"
    DEPENDENCIES std_msgs geometry_msgs diagnostic_msgs builtin_interfaces
)
ament_export_dependencies(rosidl_default_runtime rclcpp sensor_msgs geometry_msgs std_msgs diagnostic_msgs)

# --- Node executable ---
add_executable(${PROJECT_NAME}_node
    src/inertial_sense_ros2.cpp
    shared/src/inertial_sense_node.cpp
    shared/src/ParamHelper.cpp
    shared/src/RtkRover.cpp
    shared/src/RtkBase.cpp
)
target_include_directories(${PROJECT_NAME}_node PRIVATE shared/include)

rosidl_get_typesupport_target(cpp_typesupport_target "${PROJECT_NAME}" "rosidl_typesupport_cpp")
target_link_libraries(${PROJECT_NAME}_node
    ${cpp_typesupport_target}
    InertialSense::SDK
    yaml-cpp
    Threads::Threads
)
ament_target_dependencies(${PROJECT_NAME}_node
    rclcpp std_msgs geometry_msgs nav_msgs diagnostic_msgs sensor_msgs std_srvs
)

# --- gtest binary ---
ament_add_gtest(test_${PROJECT_NAME}
    src/inertial_sense_ros2.cpp
    shared/src/ParamHelper.cpp
    shared/src/RtkRover.cpp
    shared/src/RtkBase.cpp
    test/test_basic_unit_tests.cpp
    test/test_client_reconnect.cpp
    test/test_communications.cpp
    test/gtest_helpers.h
)
target_include_directories(test_${PROJECT_NAME} PRIVATE shared/include)
target_link_libraries(test_${PROJECT_NAME}
    ${cpp_typesupport_target}
    InertialSense::SDK
    yaml-cpp
    Threads::Threads
)
ament_target_dependencies(test_${PROJECT_NAME}
    rclcpp std_msgs geometry_msgs nav_msgs diagnostic_msgs sensor_msgs std_srvs
)

install(TARGETS ${PROJECT_NAME}_node test_${PROJECT_NAME}
    DESTINATION lib/${PROJECT_NAME}
)

ament_package()
