#ifndef YLM_API_CLIENT_H
#define YLM_API_CLIENT_H

#include <string>
#include <curl/curl.h>

namespace lumotive_api_client{
    const int POST = 0;
    const int GET = 1;

    class LumotiveAPIClient{
    private:
        std::string sensor_ip_;


    public:
        LumotiveAPIClient();

        void set_sensor_ip(const std::string& sensor_ip);

        std::string get_system_version();

        std::string get_sensor_id();

        std::string get_state();

        std::string post_start_scan();

        std::string post_stop_scan();

        std::string get_scan_parameters_options();

        std::string get_scan_parameters();

        std::string post_scan_parameters(const std::string& parameters);

        static size_t curl_callback(char* ptr,size_t size,size_t nmemb,std::string *responce_data);
        bool client_library_wrapper(const int method, const std::string& endpoint, const std::string& send_data, std::string &result);
    };

}



#endif