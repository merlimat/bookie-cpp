#pragma once

class ZooKeeper;
class BookieConfig;

#include <string>

/**
 * Register the bookie under /ledgers/available and makes sure the z-node is re-created after the session expires
 */
class BookieRegistration {
public:
    BookieRegistration(ZooKeeper* zk_, const BookieConfig& conf);

private:
    void handleNewZooKeeperSession();

    void registerBookie();

    ZooKeeper* zk_;
    std::string registrationPath_;
};
