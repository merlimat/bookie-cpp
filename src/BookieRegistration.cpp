#include "Bookie.h"
#include "BookieRegistration.h"
#include "Logging.h"
#include "ZooKeeper.h"

#include <folly/io/async/EventBaseManager.h>
using folly::EventBaseManager;

DECLARE_LOG_OBJECT();

BookieRegistration::BookieRegistration(ZooKeeper* zk, const BookieConfig& conf) :
        zk_(zk) {
    registrationPath_ = format("/ledgers/available/{}:{}", conf.bookieHost(), conf.bookiePort()).str();
    zk_->registerSessionListener(std::bind(&BookieRegistration::handleNewZooKeeperSession, this));
}

void BookieRegistration::handleNewZooKeeperSession() {
    LOG_INFO("Registering bookie on new ZK session");
    registerBookie();
}

void BookieRegistration::registerBookie() {
    zk_->create(registrationPath_, "", { ZooKeeper::CreateFlag::Ephemeral }) //
    .onError([this](const ZooKeeperException& e) {
        // TODO: deferred task is not working
        LOG_FATAL("Error registering bookie: " << e.what() << " -- Exiting");
        std::exit(1);
//        LOG_WARN("Error registering bookie: " << e.what() << " -- Retrying later");
//
//            EventBaseManager::get()->getEventBase()->runAfterDelay([this]() {
//                        LOG_INFO("Retrying bookie registration after error");
//                        this->registerBookie();
//                    }, std::chrono::milliseconds(10000).count());
            return std::string("");
        }) //
    .then([](std::string path) {
        if (!path.empty()) {
            LOG_INFO("Registered bookie at " << path);
        }
    });
}
