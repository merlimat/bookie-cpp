#pragma once

#include <string>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class BookieConfig {
public:
    BookieConfig();

    bool parse(int argc, char** argv);

    const std::string& zkServers() const {
        return zkServers_;
    }

    int zkSessionTimeout() const {
        return zkSessionTimeout_;
    }

    const std::string bookieHost() const {
        return bookieHost_;
    }

    int bookiePort() const {
        return bookiePort_;
    }

    const std::string& dataDirectory() const {
        return dataDirectory_;
    }

    const std::string& walDirectory() const {
        return walDirectory_;
    }

private:
    std::string zkServers_;
    int zkSessionTimeout_;

    std::string bookieHost_;
    int bookiePort_;

    std::string dataDirectory_;
    std::string walDirectory_;

    po::options_description options_;
};
