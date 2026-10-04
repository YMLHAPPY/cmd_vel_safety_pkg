import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # 1. 获取参数文件的绝对路径（这个万能公式你背下来没？）
    config_file = os.path.join(
        get_package_share_directory('cmd_vel_safety_pkg'),
        'config',
        'params.yaml'
    )

    return LaunchDescription([
        # 2. 启动“判断者（安全卫士）”，并挂载参数文件
        Node(
            package='cmd_vel_safety_pkg',
            executable='safety_filter_node',
            name='safety_filter_node',
            parameters=[config_file],  # 把 YAML 里的参数喂给它！
            output='screen'
        ),

        # 3. 启动“汇报者（状态监控）”
        Node(
            package='cmd_vel_safety_pkg',
            executable='status_monitor_node',
            name='status_monitor_node',
            output='screen'
        )
    ])