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
#include "Data/State.h"

#define LOOP_RATE 100ms     /**< loop rate. in this program, used in the api_sending_loop */
#define YLM_STARTUP_TIME_SEC 60 /**< ylm startup time */
#define LOOP_TIMEOUT_SEC 10     /**< timeout of startscan/stopscan api loop */
#define HANDLER_TIMEOUT_SEC 2   /**< timeout of api handler */

/**
 * @enum API_COMMAND_TYPE
 * @brief Command type for some operation using the API. not API type.
 * @brief APIを使用した処理のコマンドタイプ. API単体のタイプではない.
 */
enum API_COMMAND_TYPE {
    WAIT_INITIALIZATION ,
    START_SCAN ,
    STOP_SCAN ,
    POWER_OFF 
};

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

    /**
     * @brief   Some operation using API. ex) WAIT_INITIALIZATION use GetState API and return connection (successful or failure.)
     * @param[in] commandType Command type of operation
     * @param[out] none
     * @return  successful or failure.
    */
    bool APICommand(API_COMMAND_TYPE commandType)
    {
        bool b_succeed = false;
        bool b_connected = false;
        State state;
        std::string response;
        switch( commandType ) {
            case WAIT_INITIALIZATION :
                b_connected = m_handler.tryGetState(response, state);
                b_succeed = b_connected && (state.state == "ENERGIZED" || state.state == "SCANNING");
                break ;
            case START_SCAN :
                b_succeed = m_handler.tryStartScan(response);
                break ;
            case STOP_SCAN :
                b_succeed = m_handler.tryStopScan(response);
                break ;
            case POWER_OFF :
                b_succeed = m_handler.tryPostDisable(response);
                break ;
            default :
                break ;
        }
        return b_succeed;
    }


    /**
     * @brief 
     * @param[in] commandType   Command type of operation
     * @param[in] timeout       timeout of api response [sec]
     * @param[in] loopMsg       every loop message ( RCLCPP_INFO )
     * @param[in] timeOutMsg    timeout error message (RCLCPP_ERROR)
     * @param[out] none
     * @return none
    */
    void APICommandLoop(API_COMMAND_TYPE commandType, int32_t timeout, std::string loopMsg, std::string timeOutMsg)
    {
        rclcpp::WallRate loopRate(LOOP_RATE);
        bool b_succeed = false;
        bool b_timeOut = false;
        auto timeoutDuration = std::chrono::seconds(timeout);
        auto startTime = std::chrono::steady_clock::now();
        while ( !b_succeed && !b_timeOut ) {
            loopRate.sleep();
            RCLCPP_INFO(this->get_logger(), loopMsg.c_str());
            auto currentTime = std::chrono::steady_clock::now();
            b_succeed = APICommand(commandType);
            b_timeOut = (currentTime - startTime) >= timeoutDuration;
        }
        if ( !b_succeed && b_timeOut ) {
            RCLCPP_ERROR(this->get_logger(), timeOutMsg.c_str());
            throw std::runtime_error("An error has occurred. The node will be terminated.");
        }
    }

public:
    /**
     * @brief constructor
     * @param[in] none
     * @param[out] none
     * @return none
    */
    LumotiveAPIDriver(void) : Node("lumotive_api_driver"), m_handler()
    {
        // ros parameters
        std::string sensor_ip = "192.168.0.10"; /**< default value */
        this->declare_parameter("sensor_ip", sensor_ip);
        this->get_parameter("sensor_ip", sensor_ip);

        // handler setting
        m_handler.setTimeout(HANDLER_TIMEOUT_SEC);
        m_handler.setHost(sensor_ip);

        State state;
        std::string response;
        
        // waiting ylm-initialize
        API_COMMAND_TYPE commandType = WAIT_INITIALIZATION;
        std::string loopMsg = "waiting initialize...";
        std::string timeOutMsg = "ylm initialization timeout";
        APICommandLoop(commandType, YLM_STARTUP_TIME_SEC, loopMsg, timeOutMsg);
        
        // start scan
        commandType = START_SCAN;
        loopMsg = "waiting start_scan...";
        timeOutMsg = "ylm start_scan timeout";
        APICommandLoop(commandType, YLM_STARTUP_TIME_SEC, loopMsg, timeOutMsg);

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
        std::string response;

        //stop scan
        API_COMMAND_TYPE commandType = STOP_SCAN;
        std::string loopMsg = "waiting stop_scan...";
        std::string timeOutMsg = "ylm stop_scan timeout";
        APICommandLoop(commandType, YLM_STARTUP_TIME_SEC, loopMsg, timeOutMsg);


        /*
        //  以下を有効にすると、ノードの終了時にYLMセンサの電源を自動で落とします. 
        //  (ノードを落とす度に電源を再投入することが必要になるため、コメントアウトしています.)
        // power off
        commandType = POWER_OFF;
        loopMsg = "waiting power_off...";
        timeOutMsg = "ylm power_off timeout";
        APICommandLoop(commandType, YLM_STARTUP_TIME_SEC, loopMsg, timeOutMsg);
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