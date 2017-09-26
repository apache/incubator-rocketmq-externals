#ifndef _ALOG_ADAPTER_H_
#define _ALOG_ADAPTER_H_

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include "MQClient.h"

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
using namespace boost::log::trivial;
namespace rocketmq {

class logAdapter {
 public:
  ~logAdapter();
  static logAdapter& getLogInstance();
  void setLogLevel(elogLevel logLevel);
  elogLevel getLogLevel();
  void setLogFileNumAndSize(int logNum, int sizeOfPerFile);
  src::severity_logger<boost::log::trivial::severity_level>&
  getSeverityLogger() {
    return m_severityLogger;
  }

 private:
  logAdapter();
  elogLevel m_logLevel;
  std::string m_logFile;
  src::severity_logger<boost::log::trivial::severity_level> m_severityLogger;
  typedef sinks::synchronous_sink<sinks::text_file_backend> logSink_t;
  boost::shared_ptr<logSink_t> m_logSink;
};

#define ALOG_ADAPTER logAdapter::getLogInstance()

#define AGENT_LOGGER ALOG_ADAPTER.getSeverityLogger()

class LogUtil {
 public:
  static void LogMessage(boost::log::trivial::severity_level level, int line,
                        const char* format, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, format);
    boost::scoped_array<char> formattedString(new char[1024]);
    vsnprintf(formattedString.get(), 1024, format, arg_ptr);
    BOOST_LOG_SEV(AGENT_LOGGER, level) << formattedString.get();
    va_end(arg_ptr);
  }
};

#define LOG_FATAL(...) \
  LogUtil::LogMessage(boost::log::trivial::fatal, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) \
  LogUtil::LogMessage(boost::log::trivial::error, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) \
  LogUtil::LogMessage(boost::log::trivial::warning, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) \
  LogUtil::LogMessage(boost::log::trivial::info, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) \
  LogUtil::LogMessage(boost::log::trivial::debug, __LINE__, __VA_ARGS__)
}
#endif
