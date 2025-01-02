#include <iostream>
#include "httplib/httplib.h"
#include "nlohmann/json.hpp"


void printHelp()
{
    std::cout << "Ubuntu Cloud Image Helper: " << std::endl;
    std::cout << "list - list all supported Ubunutu releases" << std::endl;
    std::cout << "lts - return the current LTS release" << std::endl;
    std::cout << "sha256 <release> - return the sha256 of the disk1.img for the specified Ubuntu release (specify by version number or release name)" << std::endl;
}

int main(int argc, char * argv[]) 
{
    if( argc < 2 )
    {   
        printHelp();
        return 0;
    }
    httplib::Client cli("https://cloud-images.ubuntu.com");

    auto res = cli.Get("/releases/streams/v1/com.ubuntu.cloud:released:download.json");
    std::vector<nlohmann::json> versions;
    if (res) {  // Check if request was successful
        if (res->status == 200) {  // Check if status code is OK
            nlohmann::json j = nlohmann::json::parse(res->body);
            
            auto releases = j.find("products");
            for(auto const & r : releases.value())
            {
                if(r["arch"] == "amd64")
                {
                    //std::cout << r["release"] << std::endl;
                    versions.push_back(r);
                }
            }
        } else {
            std::cout << "HTTP error: " << res->status << std::endl;
        }
    } else {
        std::cout << "Request failed: " << res.error() << std::endl;
    }

    for (auto const & json : versions)
    {
        std::cout << json["version"] << " (" << json["release_codename"] << ")" << std::endl;
    }

    return 0;
}