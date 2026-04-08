#include "lumotive_ros2/api_client.h"
#include <iostream>

using namespace lumotive_api_client;

LumotiveAPIClient::LumotiveAPIClient(){ }

void LumotiveAPIClient::set_sensor_ip(const std::string& sensor_ip){
    sensor_ip_ = sensor_ip;
}

std::string LumotiveAPIClient::get_system_version(){
    std::string endpoint = "/system_version";
    std::string result;
    std::string dummy;
    client_library_wrapper(GET, endpoint, dummy, result);
    return result;
}

std::string LumotiveAPIClient::get_sensor_id(){
    std::string endpoint  = "/sensor_id";
    std::string result;
    std::string dummy;
    client_library_wrapper(GET, endpoint, dummy, result);
    return result;
}

std::string LumotiveAPIClient::get_state(){
    std::string endpoint  = "/state";
    std::string result;
    std::string dummy;
    client_library_wrapper(GET, endpoint, dummy, result);
    return result;
}

std::string LumotiveAPIClient::post_start_scan(){
    std::string endpoint  = "/start_scan";
    std::string result;
    std::string dummy;
    client_library_wrapper(POST, endpoint, dummy, result);
    return result;
}

std::string LumotiveAPIClient::post_stop_scan(){
    std::string endpoint  = "/stop_scan"; 
    std::string result;
    std::string dummy;
    client_library_wrapper(POST, endpoint, dummy, result);
    return result;
}

std::string LumotiveAPIClient::get_scan_parameters_options(){
    std::string endpoint  = "/scan_parameters/opts";
    std::string result;
    std::string dummy;
    client_library_wrapper(GET, endpoint, dummy, result);
    return result;
}

std::string LumotiveAPIClient::get_scan_parameters(){
    std::string endpoint  = "/scan_parameters";
    std::string result;
    std::string dummy;
    client_library_wrapper(GET, endpoint, dummy, result);
    return result;
}

std::string LumotiveAPIClient::post_scan_parameters(const std::string& parameters){
    std::string endpoint  = "/scan_parameters";
    std::string result;
    client_library_wrapper(POST, endpoint, parameters, result);
    return result;
}

size_t LumotiveAPIClient::curl_callback(char* ptr,size_t size,size_t nmemb,std::string *responce_data){
    size_t total_size=size * nmemb;
    responce_data->append(ptr,total_size);
    return total_size;
}

bool LumotiveAPIClient::client_library_wrapper(const int method, const std::string& endpoint, const std::string& send_data, std::string &result){
    std::string url = "http://" + sensor_ip_ + endpoint;

    // using libcurl.
    // （課題）通信を複数回行う場合、ハンドラを使いまわすほうがリソース的には効率的になる.
    CURL*       curl_handler = NULL;
    CURLcode    curl_code;
 
    // initialize curl.
    curl_handler = curl_easy_init();
    if (!curl_handler) {
        return false;
    }
 
    // curl option
    curl_easy_setopt(curl_handler, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handler, CURLOPT_SSL_VERIFYPEER,0);
    curl_easy_setopt(curl_handler, CURLOPT_WRITEFUNCTION, LumotiveAPIClient::curl_callback);
    curl_easy_setopt(curl_handler, CURLOPT_WRITEDATA, &result);

    if(method == GET){
        curl_easy_setopt(curl_handler, CURLOPT_HTTPGET, 1L);
    }
    if(method == POST){
        curl_easy_setopt(curl_handler, CURLOPT_POST, 1L);
        curl_easy_setopt(curl_handler, CURLOPT_POSTFIELDS, send_data.c_str());
    }

    curl_code = curl_easy_perform(curl_handler);
    if (curl_code != CURLE_OK){
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(curl_code) << std::endl;
        return false;
    }

    curl_easy_cleanup(curl_handler); 
    return true;
}

// sample
// int main() {
//     LumotiveAPIClient client;
//     client.set_sensor_ip("192.168.0.10");
    
//     std::string result;
    
//     result = client.post_start_scan();
//     std::cout << "start_scan() : " << result << std::endl;

//     return 0;
// }

