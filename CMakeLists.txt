cmake_minimum_required(VERSION 3.8)
project(cmd_vel_safety_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)

# 编译：安全卫士节点
add_executable(safety_filter_node src/safety_filter_node.cpp)
ament_target_dependencies(safety_filter_node rclcpp geometry_msgs)

# 编译：状态监控节点
add_executable(status_monitor_node src/status_monitor_node.cpp)
ament_target_dependencies(status_monitor_node rclcpp geometry_msgs std_msgs)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# 安装可执行文件
install(TARGETS
  safety_filter_node
  status_monitor_node
  DESTINATION lib/${PROJECT_NAME}
)

# 安装 launch 和 config 文件夹（还记得之前学的“文件去哪儿了”公式吗？）
install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
