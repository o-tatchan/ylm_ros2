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
#define DEFAULT_IP "192.168.0.10" /**< default value of ylm ip address */

typedef int32_t SINT32;


/**
 * @enum API_COMMAND_TYPE
 * @brief Command type for some operation using the API. not API type.
 * @brief APIを使用した処理のコマンドタイプ. API単体のタイプではない.
 */
enum class API_COMMAND_TYPE {
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
    public:
    /**
     * @brief constructor
     * @param[in] none
     * @param[out] none
     * @return none
    */
    LumotiveAPIDriver(void) : Node("lumotive_api_driver"), m_handler()
    {
        mb_apiTimeOut = false;

        // ros parameters
        std::string sensor_ip = DEFAULT_IP;
        this->declare_parameter("sensor_ip", sensor_ip);
        this->get_parameter("sensor_ip", sensor_ip);

        // handler setting
        m_handler.setTimeout(HANDLER_TIMEOUT_SEC);
        m_handler.setHost(sensor_ip);

        State state;
        std::string response;
        
        // waiting ylm-initialize
        API_COMMAND_TYPE commandType = API_COMMAND_TYPE::WAIT_INITIALIZATION;
        std::string loopMsg = "waiting initialize...";
        std::string timeOutMsg = "ylm initialization timeout";
        bool b_commandSuccess = APICommandLoop(commandType, YLM_STARTUP_TIME_SEC, loopMsg, timeOutMsg);
        if ( !b_commandSuccess ) {
            return;
        }

        // start scan
        commandType = API_COMMAND_TYPE::START_SCAN;
        loopMsg = "waiting start_scan...";
        timeOutMsg = "ylm start_scan timeout";
        b_commandSuccess = APICommandLoop(commandType, LOOP_TIMEOUT_SEC, loopMsg, timeOutMsg);
        if ( !b_commandSuccess ) {
            return;
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
        std::string response;

        //stop scan
        API_COMMAND_TYPE commandType = API_COMMAND_TYPE::STOP_SCAN;
        std::string loopMsg = "waiting stop_scan...";
        std::string timeOutMsg = "ylm stop_scan timeout";
        bool b_commandSuccess = APICommandLoop(commandType, LOOP_TIMEOUT_SEC, loopMsg, timeOutMsg);
        if ( !b_commandSuccess ) {
            return;
        }

        /*
        //  以下を有効にすると、ノードの終了時にYLMセンサの電源を自動で落とします. 
        //  (ノードを落とす度に電源を再投入することが必要になるため、コメントアウトしています.)
        // power off
        commandType = API_COMMAND_TYPE::POWER_OFF;
        loopMsg = "waiting power_off...";
        timeOutMsg = "ylm power_off timeout";
        b_commandSuccess = APICommandLoop(commandType, LOOP_TIMEOUT_SEC, loopMsg, timeOutMsg);
        if ( !b_commandSuccess ) {
            return;
        }
        */
        
        RCLCPP_INFO(this->get_logger(), "finish");
    }

    /**
     * @brief shut down this ros-node when any command times out.
     * @brief should be called after constructor.
     * @param[in] none
     * @param[out] none
     * @return none
    */
    void ShutdownIfTimeOut(void)
    {
        if( mb_apiTimeOut ) {
            rclcpp::shutdown();
        }
    }

private:
    Handler m_handler; /**< ylm api handler */
    bool mb_apiTimeOut; /**< error flag */

    /**
     * @brief   Some operation using API. ex) WAIT_INITIALIZATION use GetState API and return connection (successful or failure.)
     * @param[in] commandType Command type of operation
     * @param[out] none
     * @return  successful or failure.
    */
    bool APICommand(API_COMMAND_TYPE commandType)
    {
        bool b_success = false;
        bool b_connect = false;
        State state;
        std::string response;
        switch( commandType ) {
            case API_COMMAND_TYPE::WAIT_INITIALIZATION :
                b_connect = m_handler.tryGetState(response, state);
                b_success = b_connect && (state.state == "ENERGIZED" || state.state == "SCANNING");
                break ;
            case API_COMMAND_TYPE::START_SCAN :
                m_handler.tryStartScan(response);
                b_connect = m_handler.tryGetState(response, state);
                b_success = b_connect && (state.state == "SCANNING");
                break ;
            case API_COMMAND_TYPE::STOP_SCAN :
                m_handler.tryStopScan(response);
                b_connect = m_handler.tryGetState(response, state);
                b_success = b_connect && (state.state == "ENERGIZED");
                break ;
            case API_COMMAND_TYPE::POWER_OFF :
                b_success = m_handler.tryPostDisable(response);
                break ;
            default :
                break ;
        }
        return b_success;
    }


    /**
     * @brief 
     * @param[in] commandType   Command type of operation
     * @param[in] timeout       timeout of api response [sec]
     * @param[in] loopMsg       every loop message ( RCLCPP_INFO )
     * @param[in] timeOutMsg    timeout error message (RCLCPP_ERROR)
     * @param[out] none
     * @return successful or failure.
    */
    bool APICommandLoop(API_COMMAND_TYPE commandType, SINT32 timeout, std::string loopMsg, std::string timeOutMsg)
    {
        rclcpp::WallRate loopRate(LOOP_RATE);
        bool b_success = false;
        bool b_timeOut = false;
        auto timeoutDuration = std::chrono::seconds(timeout);
        auto startTime = std::chrono::steady_clock::now();
        while ( !b_success && !b_timeOut ) {
            loopRate.sleep();
            RCLCPP_INFO(this->get_logger(), loopMsg.c_str());
            auto currentTime = std::chrono::steady_clock::now();
            b_success = APICommand(commandType);
            b_timeOut = (currentTime - startTime) >= timeoutDuration;
        }
        if ( !b_success && b_timeOut ) {
            RCLCPP_ERROR(this->get_logger(), timeOutMsg.c_str());
            mb_apiTimeOut = true;
            return false;
        }
        return true;
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
    auto node = std::make_shared<LumotiveAPIDriver>();
    node->ShutdownIfTimeOut();
    if (!rclcpp::ok()) {
        return 1;  // shutdown 済みなら終了.
    }

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}