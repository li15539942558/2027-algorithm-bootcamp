// subscriber_node.cpp —— 订阅者节点（任务一）
// 功能：订阅 "bootcamp_topic"，收到数据后用 Logger 打印出接收到的数据
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

class SubscriberNode : public rclcpp::Node {
public:
    SubscriberNode() : Node("listener_node") {
        // 创建订阅者：话题名与发布者一致，队列长度 10
        subscription_ = this->create_subscription<std_msgs::msg::String>(
            "bootcamp_topic", 10,
            std::bind(&SubscriberNode::onMessage, this, std::placeholders::_1));

        RCLCPP_INFO(this->get_logger(), "订阅者节点已启动, 正在监听话题: %s", "bootcamp_topic");
    }

private:
    void onMessage(const std_msgs::msg::String::SharedPtr msg) const {
        // 用 Logger 打印接收到的数据（任务要求的核心）
        RCLCPP_INFO(this->get_logger(), "收到数据: \"%s\"", msg->data.c_str());
    }

    rclcpp::Subscription<std_msgs::msg::String>::SharedPtr subscription_;
};

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<SubscriberNode>());
    rclcpp::shutdown();
    return 0;
}
