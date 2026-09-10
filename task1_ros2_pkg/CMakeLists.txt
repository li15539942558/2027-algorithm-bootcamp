cmake_minimum_required(VERSION 3.8)
project(rm_bootcamp_pubsub)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# 发布者节点
add_executable(talker_node src/publisher_node.cpp)
ament_target_dependencies(talker_node rclcpp std_msgs)

# 订阅者节点
add_executable(listener_node src/subscriber_node.cpp)
ament_target_dependencies(listener_node rclcpp std_msgs)

install(TARGETS
  talker_node
  listener_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
