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

#define ROS_LOOP_RATE 100ms    /**< loop rate. in this program, used in the api_sending_loop */
#define LOOP_TIMEOUT_SEC 30     /**< timeout of sending api loop */
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

        rclcpp::WallRate loop_rate(ROS_LOOP_RATE);
        State state;
        std::string response;
        
        bool b_initialized = false;
        while ( !b_initialized ) {
            loop_rate.sleep();
            RCLCPP_INFO(this->get_logger(), "waiting initialize...");
            bool b_connected = m_handler.tryGetState(response, state);
            b_initialized = b_connected && state.state!="INITIALIZE";
        }
        
        RCLCPP_INFO(this->get_logger(), "waiting start_scan...");
        while (!m_handler.tryStartScan(response)) {
            loop_rate.sleep();
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
        rclcpp::WallRate loop_rate(ROS_LOOP_RATE);
        std::string response;

        RCLCPP_INFO(this->get_logger(), "waiting stop_scan...");
        while (!m_handler.tryStopScan(response)) {
            loop_rate.sleep();
        }

        /*
         以下を有効にすると、ノードの終了時にYLMセンサの電源を自動で落とします. 
         (ノードを落とす度に電源を再投入することが必要になるため、コメントアウトしています.)
        while (!m_handler.tryPostDisable(response)) {
            loop_rate.sleep();
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