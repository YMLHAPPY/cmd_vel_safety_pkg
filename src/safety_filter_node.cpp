#include <chrono>
#include <memory>
#include <functional>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"      // 输入/输出的消息类型

using namespace std::chrono_literals;

class SafetyFilterNode : public rclcpp::Node
{
public:
  SafetyFilterNode() : Node("safety_filter_node")
  {
    // 1. 声明参数（三根可调的旋钮）
    max_linear_speed_ = this->declare_parameter<double>("max_linear_speed", 1.0);
    max_angular_speed_ = this->declare_parameter<double>("max_angular_speed", 1.5);
    cmd_vel_timeout_ = this->declare_parameter<double>("cmd_vel_timeout", 1.0);

    // 初始化上次收到消息的时间（用当前时间）
    last_msg_time_ = this->now();

    // 2. 创建订阅者：监听原始速度 /cmd_vel
    subscription_ = this->create_subscription<geometry_msgs::msg::Twist>(
      "/cmd_vel", rclcpp::SensorDataQoS(), std::bind(&SafetyFilterNode::cmd_vel_callback, this, std::placeholders::_1));

    // 3. 创建发布者：发布处理后安全的速度 /cmd_vel_safe
    publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel_safe", 10);

    // 4. 创建定时器：每 100ms 检查一次是否超时
    timer_ = this->create_wall_timer(
      100ms, std::bind(&SafetyFilterNode::check_timeout, this));

    RCLCPP_INFO(this->get_logger(), "安全卫士节点已启动，监听 /cmd_vel");
  }

private:
  void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
  {
    // 收到消息，刷新“最后一次”的时间戳
    last_msg_time_ = this->now();

    auto safe_msg = geometry_msgs::msg::Twist();

    // --- 【核心判断逻辑 1】线速度限幅 ---
    // 如果原始速度 > 最大允许速度，就砍到最大允许值；低于负极限也同理。
    if (msg->linear.x > max_linear_speed_) {
      safe_msg.linear.x = max_linear_speed_;
    } else if (msg->linear.x < -max_linear_speed_) {
      safe_msg.linear.x = -max_linear_speed_;
    } else {
      safe_msg.linear.x = msg->linear.x;
    }

    // --- 【核心判断逻辑 2】角速度限幅 ---
    if (msg->angular.z > max_angular_speed_) {
      safe_msg.angular.z = max_angular_speed_;
    } else if (msg->angular.z < -max_angular_speed_) {
      safe_msg.angular.z = -max_angular_speed_;
    } else {
      safe_msg.angular.z = msg->angular.z;
    }

    // 将处理好的安全指令发出去
    publisher_->publish(safe_msg);
    
    RCLCPP_INFO(this->get_logger(), "原始: %.2f, %.2f -> 安全: %.2f, %.2f", 
                msg->linear.x, msg->angular.z, safe_msg.linear.x, safe_msg.angular.z);
  }

  // --- 【核心判断逻辑 3】异常兜底（超时刹车） ---
  void check_timeout()
  {
    // 算一下距离上一次收到消息，过了多少秒
    auto time_since_last_msg = this->now() - last_msg_time_;
    
    // 如果超过预设的 timeout 时间
    if (time_since_last_msg.seconds() > cmd_vel_timeout_) {
      // 立即发布一个全零的 Twist（刹车！）
      auto zero_msg = geometry_msgs::msg::Twist();
      publisher_->publish(zero_msg);
      
      RCLCPP_WARN(this->get_logger(), "超时！未收到 /cmd_vel，发送刹车指令！");
      
      // 把时间重置，防止警告信息疯狂刷屏
      last_msg_time_ = this->now(); 
    }
  }

  // 成员变量声明
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscription_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
  rclcpp::TimerBase::SharedPtr timer_;
  
  double max_linear_speed_;
  double max_angular_speed_;
  double cmd_vel_timeout_;
  rclcpp::Time last_msg_time_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<SafetyFilterNode>());
  rclcpp::shutdown();
  return 0;
}