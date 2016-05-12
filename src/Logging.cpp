#include "Logging.h"

#include <log4cxx/logger.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <iostream>

using namespace log4cxx;

void Logging::init() {
    Logging::init("");
}

void Logging::init(const std::string& logfilePath) {
    try {
        if (logfilePath.empty()) {
            if (!LogManager::getLoggerRepository()->isConfigured()) {
                LogManager::getLoggerRepository()->setConfigured(true);
                LoggerPtr root = Logger::getRootLogger();
                static const LogString TTCC_CONVERSION_PATTERN(LOG4CXX_STR("%d{HH:mm:ss.SSS} [%t] %-5p %l - %m%n"));
                LayoutPtr layout(new PatternLayout(TTCC_CONVERSION_PATTERN));
                AppenderPtr appender(new ConsoleAppender(layout));
                root->setLevel(log4cxx::Level::getInfo());
                root->addAppender(appender);
            }
        } else {
            log4cxx::PropertyConfigurator::configure(logfilePath);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "exception caught while configuring log4cpp via '" << logfilePath << "': " << e.what()
                << std::endl;
    }
    catch (...) {
        std::cerr << "unknown exception while configuring log4cpp via '" << logfilePath << "'." << std::endl;
    }
}

