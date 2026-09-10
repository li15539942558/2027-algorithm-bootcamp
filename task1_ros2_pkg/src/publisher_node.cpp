// publisher_node.cpp —— 发布者节点（任务一）
// 功能：周期性发布自定义内容（这里用 std_msgs::msg::String 发送一个带计数与时间戳的消息）
#include <chrono>
#include <memory>
#include <string>
#include <sstream>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

class PublisherNode : public rclcpp::Node {
public:
    PublisherNode() : Node("talker_node"), count_(0) {
        // 创建发布者：话题名 "bootcamp_topic"，队列长度 10
        publisher_ = this->create_publisher<std_msgs::msg::String>("bootcamp_topic", 10);

        // 每 500 ms 发布一次
        timer_ = this->create_wall_timer(500ms, std::bind(&PublisherNode::onTimer, this));

        RCLCPP_INFO(this->get_logger(), "发布者节点已启动, 话题: %s", "bootcamp_topic");
    }

private:
    void onTimer() {
        auto message = std_msgs::msg::String();
        ++count_;

        std::ostringstream oss;
        oss << "2027赛季算法组竞培营 第 " << count_ << " 条消息 | 时间戳: "
            << this->now().seconds();
        message.data = oss.str();

        publisher_->publish(message);
        // 发布日志（Logger）
        RCLCPP_INFO(this->get_logger(), "已发布: \"%s\"", message.data.c_str());
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    size_t count_;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<PublisherNode>());
    rclcpp::shutdown();
    return 0;
}
