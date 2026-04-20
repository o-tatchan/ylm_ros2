#!/usr/bin/env python

from launch import LaunchDescription
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.conditions import IfCondition

from ament_index_python.packages import get_package_share_directory

import os
import yaml

def generate_launch_description():
    # launch argument
    ld = LaunchDescription([
        DeclareLaunchArgument('sensor_ip_', default_value="192.168.0.10", description='IP of Lumotive sensor to connect to.'),
        DeclareLaunchArgument('sensor_port_', default_value="10940", description='Port of Lumotive sensor to connect to.'),
        DeclareLaunchArgument('launch_api_client_', default_value="true", description='Launch flag of api client. if true, YLM auto start scan.'),
        DeclareLaunchArgument('launch_rviz_', default_value="true", description='Launch flag of rviz.'),
    ])
    
    # load yaml file
    config_path = os.path.join(
        get_package_share_directory('ylm_ros2'),
        'config',
        'm20_configs.yaml'
    )

    yaml_configs = yaml.safe_load(open(config_path, 'r'))
    configs = yaml_configs['lumotive_ros_params']
    configs['sensor_ip'] = LaunchConfiguration('sensor_ip_')
    configs['sensor_port'] = LaunchConfiguration('sensor_port_')
    
    drv_node = Node(
        package='ylm_ros2',
        executable='lumotive_driver',
        output='screen',
        parameters = [configs]
        )
    ld.add_action(drv_node)
    
    api_node = Node(
        package='ylm_ros2',
        executable='api_driver',
        output='screen',
        parameters = [configs],
        condition = IfCondition(LaunchConfiguration('launch_api_client_'))
        )
    ld.add_action(api_node)

    rviz = Node(
        package='rviz2',
        namespace='',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', [os.path.join(get_package_share_directory('ylm_ros2'), 'rviz', 'm20_default.rviz')]],
        condition=IfCondition(LaunchConfiguration('launch_rviz_'))
        )        
    ld.add_action(rviz)

    return ld