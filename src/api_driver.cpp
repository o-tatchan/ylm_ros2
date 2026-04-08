#include "lumotive_ros2/api_client.h"
#include "rclcpp/rclcpp.hpp"


class LumotiveAPIDriver : public rclcpp::Node {
private:
    lumotive_api_client::LumotiveAPIClient api_client;



public:
    LumotiveAPIDriver() : Node("lumotive_api_driver"){
        std::string sensor_ip = "192.168.0.10";
        this->declare_parameter("sensor_ip", sensor_ip);
        this->get_parameter("sensor_ip", sensor_ip);
        api_client.set_sensor_ip(sensor_ip);

        api_client.post_start_scan();
    }

    ~LumotiveAPIDriver(){
        api_client.post_stop_scan();
    }

};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LumotiveAPIDriver>());
    rclcpp::shutdown();
    return 0;
}