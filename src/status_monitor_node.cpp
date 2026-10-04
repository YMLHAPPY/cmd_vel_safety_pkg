#include <chrono>
#include <memory>
#include <functional>
#include <string>
#include <cmath> // 为了使用 std::abs 绝对值函数

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"      // 输入：接收速度消息
#include "std_msgs/msg/string.hpp"          // 输出：发送文字状态消息

class StatusMonitorNode : public rclcpp::Node
{
public:
  StatusMonitorNode() : Node("status_monitor_node")
  {
    // 1. 创建订阅者：监听“判断者”发出来的安全速度话题 /cmd_vel_safe
    subscription_ = this->create_subscription<geometry_msgs::msg::Twist>(
      "/cmd_vel_safe", 10, std::bind(&StatusMonitorNode::cmd_vel_callback, this, std::placeholders::_1));

    // 2. 创建发布者：把机器人状态发布到 /robot_status 话题
    publisher_ = this->create_publisher<std_msgs::msg::String>("/robot_status", 10);

    RCLCPP_INFO(this->get_logger(), "状态监控节点已启动，监听 /cmd_vel_safe");
  }

private:
  void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    // 1. 提取数据（因为 msg 是共享指针，所以用 -> 掏里面的字段）
    double linear = msg->linear.x;
    double angular = msg->angular.z;

    // 2. 核心判断逻辑：速度接近0就是停止，否则就是移动
    std::string status = "停止";
    if (std::abs(linear) > 0.01 || std::abs(angular) > 0.01) {
      status = "移动中";
    }

    // 3. 组装字符串（把数字拼成一段人类能看懂的话）
    std::string status_str = "当前线速度: " + std::to_string(linear) + 
                             ", 角速度: " + std::to_string(angular) + 
                             ", 状态: " + status;

    // 4. 创建 String 消息对象，填入拼好的话
    auto status_msg = std_msgs::msg::String();
    status_msg.data = status_str;

    // 5. 发布消息
    publisher_->publish(status_msg);

    // 6. 同时在终端打印出来，方便你调试时直接看
    RCLCPP_INFO(this->get_logger(), "汇报状态: %s", status_str.c_str());
  }

  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
  rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<StatusMonitorNode>());
  rclcpp::shutdown();
  return 0;
}