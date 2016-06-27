/**
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */
#pragma once

#include <log4cxx/logger.h>
#include <string>

#define DECLARE_LOG_OBJECT()                                \
    static log4cxx::LoggerPtr& logger()                     \
    {                                                       \
        static log4cxx::LoggerPtr result = log4cxx::Logger::getLogger("bookie." __FILE__);     \
        return result;                                      \
    } \


#define LOG_DEBUG(message) { \
        if (LOG4CXX_UNLIKELY(logger()->isDebugEnabled())) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger()->forcedLog(::log4cxx::Level::getDebug(), oss_.str(((std::ostream&)oss_) << message), LOG4CXX_LOCATION); }}

#define LOG_INFO(message) { \
        if (logger()->isInfoEnabled()) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger()->forcedLog(::log4cxx::Level::getInfo(), oss_.str(((std::ostream&)oss_) << message), LOG4CXX_LOCATION); }}

#define LOG_WARN(message) { \
        if (LOG4CXX_UNLIKELY(logger()->isWarnEnabled())) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger()->forcedLog(::log4cxx::Level::getWarn(), oss_.str(((std::ostream&)oss_) << message), LOG4CXX_LOCATION); }}

#define LOG_ERROR(message) { \
        if (LOG4CXX_UNLIKELY(logger()->isErrorEnabled())) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger()->forcedLog(::log4cxx::Level::getError(), oss_.str(((std::ostream&)oss_) << message), LOG4CXX_LOCATION); }}

#define LOG_FATAL(message) { \
        if (LOG4CXX_UNLIKELY(logger()->isFatalEnabled())) {\
           ::log4cxx::helpers::MessageBuffer oss_; \
           logger()->forcedLog(::log4cxx::Level::getFatal(), oss_.str(((std::ostream&)oss_) << message), LOG4CXX_LOCATION); }}

class Logging {
public:
    static void init();
    static void init(const std::string& logConfFilePath);
};

