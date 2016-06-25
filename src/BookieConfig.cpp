#include "BookieConfig.h"
#include <iostream>

#include <unistd.h>

BookieConfig::BookieConfig() :
        zkServers_(),
        zkSessionTimeout_(0),
        bookiePort_(),
        dataDirectory_(),
        walDirectory_(),
        options_("Allowed options", 100) {

    char defaultHostname[256];
    if (gethostname(defaultHostname, sizeof(defaultHostname)) != 0) {
        throw std::runtime_error("Failed to get default hostname");
    }

    options_.add_options() //
    ("help,h", "This help message") //
    ("zkServers,z", po::value<std::string>(&zkServers_)->default_value("localhost:2181"), "List of ZooKeeper servers") //
    ("zkSessionTimeout", po::value<int>(&zkSessionTimeout_)->default_value(30000), "ZooKeeper session timeout") //
    ("bookieHost", po::value<std::string>(&bookieHost_)->default_value(defaultHostname), "Boookie hostname") //
    ("bookiePort,p", po::value<int>(&bookiePort_)->default_value(3181), "Bookie TCP port") //
    ("dataDir,d", po::value<std::string>(&dataDirectory_)->default_value("./data"), "Location where to store data") //
    ("walDir,w", po::value<std::string>(&walDirectory_)->default_value("./wal"),
            "Location where to put RocksDB Write-ahead-log") //
    ("fsyncWal,s", po::value<bool>(&fsyncWal_)->default_value(true), "Fsync the WAL before acking the entry") //
            //
            ;
}

bool BookieConfig::parse(int argc, char** argv) {
    po::variables_map map;
    try {
        po::store(po::command_line_parser(argc, argv).options(options_).run(), map);
        po::notify(map);

        if (map.count("help")) {
            std::cerr << options_ << std::endl;
            exit(1);
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error parsing parameters -- " << e.what() << std::endl << std::endl;
        std::cerr << options_ << std::endl;
        return false;
    }
}

