#include "rclcpp/rclcpp.hpp"
#include <chrono>

#include "YlmHandler.h"
#include "Data/ScanParameterData.h"
#include "Data/SystemVersionData.h"
#include "Data/AngleRangeData.h"
#include "Data/MaxRangeData.h"
#include "Data/ScanParameterOpt.h"
#include "Data/State.h"
#include "Data/SensorId.h"
#include "Data/PersistentSettings.h"
#include "Data/TimeSyncStatus.h"

using namespace std::chrono_literals;
using namespace YlmConfigurator;
using namespace boost::beast::http;
using HttpResponse = boost::beast::http::response<boost::beast::http::string_body>;


class LumotiveAPIDriver : public rclcpp::Node {
private:
    Handler handler;

public:
    LumotiveAPIDriver() : Node("lumotive_api_driver"){
        std::string sensor_ip = "192.168.0.10";
        this->declare_parameter("sensor_ip", sensor_ip);
        this->get_parameter("sensor_ip", sensor_ip);
        
        handler.setTimeout(2);
        handler.setHost(sensor_ip);
        
        
        State state;
        rclcpp::WallRate loop_rate(100ms);
        std::string response;
        bool is_initialized = false;
        while( !is_initialized ){
            loop_rate.sleep();
            RCLCPP_INFO(this->get_logger(), "waiting initialize...");
            bool connection = handler.tryGetState(response, state);
            is_initialized = connection && state.state!="INITIALIZE";
        }
        
        RCLCPP_INFO(this->get_logger(), "waiting start_scan...");
        while(!handler.tryStartScan(response)) loop_rate.sleep();

        RCLCPP_INFO(this->get_logger(), "scanning start !!");
    }

    ~LumotiveAPIDriver(){
        rclcpp::WallRate loop_rate(100ms);
        std::string response;

        RCLCPP_INFO(this->get_logger(), "waiting stop_scan...");
        while(!handler.tryStopScan(response)) loop_rate.sleep();

        // RCLCPP_INFO(this->get_logger(), "waiting power_off...");
        // while(!handler.tryPostDisable(response)) loop_rate.sleep();

        RCLCPP_INFO(this->get_logger(), "finish");
    }

};

int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LumotiveAPIDriver>());
    rclcpp::shutdown();
    return 0;
}