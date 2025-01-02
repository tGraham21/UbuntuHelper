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

bool equalIgnoreCase(std::string const & a, std::string const & b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), 
    [](char a, char b) {
        return std::tolower(a) == std::tolower(b);
    });
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
    if (res) { 
        if (res->status == 200) {  
            nlohmann::json j = nlohmann::json::parse(res->body);
            
            auto releases = j.find("products");
            for(auto const & r : releases.value())
            {
                if(r["arch"] == "amd64")
                    versions.push_back(r);
            }
        } 
        else 
        {
            std::cout << "HTTP error: " << res->status << std::endl;
            return 1;
        }
    } 
    else 
    {
        std::cout << "Request failed: " << res.error() << std::endl;
        return 1;
    }

    std::string arg = argv[1];
    ///TODO(Trevor): Fix this to check 'supported'
    if(equalIgnoreCase(arg, "list"))
    {
        for (auto const & json : versions)
            std::cout << json["version"] << " (" << json["release_codename"] << ")" << std::endl;
    }
    else if (equalIgnoreCase(arg, "lts"))
    {
        double max = 0;
        std::string currLTS;
        for(auto const & json : versions)
        {
            std::string title = json["release_title"];
            if(equalIgnoreCase(title.substr(title.size() - 3), "LTS"))
            {
                std::string v = json["version"];
                double curr = std::stod(v);
                std::cout << std::to_string(curr) << std::endl;
                if(curr > max)
                {
                    currLTS = v + " (";
                    currLTS += json["release_codename"];
                    currLTS += ")";
                    max = curr;
                }
            }
        }
        if(!currLTS.empty())
            std::cout << "Current Ubuntu LTS: " << currLTS << std::endl;
        else
            std::cout << "No LTS found" << std::endl;
    }
    else if (equalIgnoreCase(arg, "sha256"))
    {
        if(argc < 3)
        {
            std::cout << "Please specifiy a release version" << std::endl;
            return 1;
        }

        std::string release = argv[2];
        for(auto const & json : versions)
        {
            // look up by any alias instead?
            if(json["version"] == release || equalIgnoreCase(json["release_codename"], release) || equalIgnoreCase(json["release"], release))
            {
                auto v = json["versions"];
                ///TODO(Trevor): clean this up
                auto disk1 = v.front()["items"]["disk1.img"];
                if(disk1.contains("sha256"))
                {
                    std::cout << disk1["sha256"] << std::endl;
                }
                else
                    std::cout << "No sha256 for specified version: " << release << std::endl;
            }
        }
    }
    else
    {
        printHelp();
    }

    return 0;
}