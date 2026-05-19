/**
 * @file api_driver.cpp
 * @brief ROS2 node of ylm api crient
 * @date 2026/05/19
 * @author okamoto
 * @details
*/

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

#define ROS_LOOP_RATE 100ms     /**< loop rate. in this program, used in the api_sending_loop */
#define YLM_STARTUP_TIME_SEC 60 /**< ylm startup time */
#define LOOP_TIMEOUT_SEC 10     /**< timeout of startscan/stopscan api loop */
#define HANDLER_TIMEOUT_SEC 2   /**< timeout of api handler */

using namespace std::chrono_literals;
using namespace YlmConfigurator;
using namespace boost::beast::http;
using HttpResponse = boost::beast::http::response<boost::beast::http::string_body>;

/**
 * @class LumotiveAPIDriver
 * @brief ROS2 node class of ylm api crient 
*/
class LumotiveAPIDriver : public rclcpp::Node {
private:
    Handler m_handler; /**< ylm api handler */

public:
    /**
     * @brief constructor
     * @param[in] none
     * @param[out] none
     * @return none
    */
    LumotiveAPIDriver(void) : Node("lumotive_api_driver")
    {
        // ros parameters
        std::string sensor_ip = "192.168.0.10"; /**< default value */
        this->declare_parameter("sensor_ip", sensor_ip);
        this->get_parameter("sensor_ip", sensor_ip);

        // handler setting
        m_handler = Handler();
        m_handler.setTimeout(HANDLER_TIMEOUT_SEC);
        m_handler.setHost(sensor_ip);

        rclcpp::WallRate loopRate(ROS_LOOP_RATE);
        State state;
        std::string response;
        
        // waiting ylm-initialize
        bool b_initialized = false;
        bool b_timeOut = false;
        auto timeoutDuration = std::chrono::seconds(YLM_STARTUP_TIME_SEC);
        auto startTime = std::chrono::steady_clock::now();
        while ( !b_initialized && !b_timeOut ) {
            loopRate.sleep();
            RCLCPP_INFO(this->get_logger(), "waiting initialize...");
            auto currentTime = std::chrono::steady_clock::now();
            bool b_connected = m_handler.tryGetState(response, state);
            b_initialized = b_connected && (state.state == "ENERGIZED" || state.state == "SCANNING");
            b_timeOut = (currentTime - startTime) >= timeoutDuration;
        }
        if ( !b_initialized && b_timeOut ) {
            RCLCPP_ERROR(this->get_logger(), "ylm initialization timeout");
            rclcpp::shutdown();
        }
        
        // start scan
        b_timeOut = false;
        bool b_startScan = false;
        timeoutDuration = std::chrono::seconds(LOOP_TIMEOUT_SEC);
        startTime = std::chrono::steady_clock::now();
        while ( !b_startScan && !b_timeOut ) {
            loopRate.sleep();
        RCLCPP_INFO(this->get_logger(), "waiting start_scan...");
            auto currentTime = std::chrono::steady_clock::now();
            b_startScan = m_handler.tryStartScan(response);
            b_timeOut = (currentTime - startTime) >= timeoutDuration;
        }
        if ( !b_startScan && b_timeOut ) {
            RCLCPP_ERROR(this->get_logger(), "ylm start_scan timeout");
            rclcpp::shutdown();
        }

        RCLCPP_INFO(this->get_logger(), "scanning start !!");
    }

    /**
     * @brief destructor
     * @param[in] none
     * @param[out] none
     * @return none
    */
    ~LumotiveAPIDriver(void)
    {
        rclcpp::WallRate loopRate(ROS_LOOP_RATE);
        std::string response;

        //stop scan
        bool b_stopScan = false;
        bool b_timeOut = false;
        auto timeoutDuration = std::chrono::seconds(LOOP_TIMEOUT_SEC);
        auto startTime = std::chrono::steady_clock::now();
        while ( !b_stopScan && !b_timeOut ) {
            loopRate.sleep();
        RCLCPP_INFO(this->get_logger(), "waiting stop_scan...");
            auto currentTime = std::chrono::steady_clock::now();
            b_stopScan = m_handler.tryStopScan(response);
            b_timeOut = (currentTime - startTime) >= timeoutDuration;
        }
        if ( !b_stopScan && b_timeOut ) {
            RCLCPP_ERROR(this->get_logger(), "ylm stop_scan timeout");
            rclcpp::shutdown();
        }

        /*
         以下を有効にすると、ノードの終了時にYLMセンサの電源を自動で落とします. 
         (ノードを落とす度に電源を再投入することが必要になるため、コメントアウトしています.)
        // power off
        b_timeOut = false;
        bool b_powerOff = false;
        startTime = std::chrono::steady_clock::now();
        while ( !b_powerOff && !b_timeOut ) {
            loopRate.sleep();
            RCLCPP_INFO(this->get_logger(), "waiting power_off...");
            auto currentTime = std::chrono::steady_clock::now();
            b_powerOff = m_handler.tryPostDisable(response);
            b_timeOut = (currentTime - startTime) >= timeoutDuration;
        }
        if ( !b_powerOff && b_timeOut ) {
            RCLCPP_ERROR(this->get_logger(), "ylm start_scan timeout");
            rclcpp::shutdown();
        }
        */

        RCLCPP_INFO(this->get_logger(), "finish");
    }

};

/**
 * @brief main function. run rosnode.
 * @param[in] argc number of ros arguments
 * @param[in] argv ros arguments
 * @return successful or failure.
*/
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<LumotiveAPIDriver>());
    rclcpp::shutdown();
    return 0;
}