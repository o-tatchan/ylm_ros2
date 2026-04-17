#include "lumotive_ros2/api_client.h"
#include "rclcpp/rclcpp.hpp"
#include <chrono>
#include <string>

using namespace std::chrono_literals;

class LumotiveAPIDriver : public rclcpp::Node {
private:
    lumotive_api_client::LumotiveAPIClient api_client;



public:
    LumotiveAPIDriver() : Node("lumotive_api_driver"){
        std::string sensor_ip = "192.168.0.10";
        this->declare_parameter("sensor_ip", sensor_ip);
        this->get_parameter("sensor_ip", sensor_ip);
        api_client.set_sensor_ip(sensor_ip);

        rclcpp::WallRate loop_rate(100ms);
        std::string ylm_state = api_client.get_state();
        while(ylm_state.find("INITIALIZED")!=std::string::npos){
            ylm_state = api_client.get_state();
            loop_rate.sleep();
        }
        while(ylm_state.find("SCANNING")!=std::string::npos){
            api_client.post_start_scan();
            loop_rate.sleep();
            ylm_state = api_client.get_state();
        }
    }

    ~LumotiveAPIDriver(){
        api_client.post_stop_scan();
        api_client.post_disable();
    }

};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LumotiveAPIDriver>());
    rclcpp::shutdown();
    return 0;
}