/********************************************************************
author:  qiwei.wqw@alibaba-inc.com
*********************************************************************/
#ifndef __UTILALL_H__
#define __UTILALL_H__

#include <assert.h>
#include <errno.h>
#include <pwd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <zlib.h>
#include <boost/asio.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale/conversion.hpp>
#include <boost/locale/encoding.hpp>
#include <sstream>

using namespace std;
namespace metaq {
//<!************************************************************************
const string WHITESPACE = " \t\r\n";
const int MASTER_ID = 0;
const string SUB_ALL = "*";
const string DEFAULT_TOPIC = "TBW102";
const string BENCHMARK_TOPIC = "BenchmarkTest";
const string DEFAULT_PRODUCER_GROUP = "DEFAULT_PRODUCER";
const string DEFAULT_CONSUMER_GROUP = "DEFAULT_CONSUMER";
const string TOOLS_CONSUMER_GROUP = "TOOLS_CONSUMER";
const string CLIENT_INNER_PRODUCER_GROUP = "CLIENT_INNER_PRODUCER";
const string SELF_TEST_TOPIC = "SELF_TEST_TOPIC";
const string RETRY_GROUP_TOPIC_PREFIX = "%RETRY%";
const string DLQ_GROUP_TOPIC_PREFIX = "%DLQ%";
const string ROCKETMQ_HOME_ENV = "ROCKETMQ_HOME";
const string ROCKETMQ_HOME_PROPERTY = "rocketmq.home.dir";
const string MESSAGE_COMPRESS_LEVEL = "rocketmq.message.compressLevel";
const int POLL_NAMESERVER_INTEVAL = 1000 * 30;
const int HEARTBEAT_BROKER_INTERVAL = 1000 * 30;
const int PERSIST_CONSUMER_OFFSET_INTERVAL = 1000 * 5;
const string WS_ADDR = "please set nameserver domain by setDomainName, there is no default nameserver domain";

const int LINE_SEPARATOR = 1;  // metaq::UtilAll::charToString((char) 1);
const int WORD_SEPARATOR = 2;  // metaq::UtilAll::charToString((char) 2);

const int HTTP_TIMEOUT = 3000;  // 3S
const int HTTP_CONFLICT = 409;
const int HTTP_OK = 200;
const int HTTP_NOTFOUND = 404;
const int CONNETERROR = -1;
const string null = "";

/** A platform-independent 8-bit signed integer type. */
typedef signed char int8;
/** A platform-independent 8-bit unsigned integer type. */
typedef unsigned char uint8;
/** A platform-independent 16-bit signed integer type. */
typedef signed short int16;
/** A platform-independent 16-bit unsigned integer type. */
typedef unsigned short uint16;
/** A platform-independent 32-bit signed integer type. */
typedef signed int int32;
/** A platform-independent 32-bit unsigned integer type. */
typedef unsigned int uint32;
/** A platform-independent 64-bit integer type. */
typedef long long int64;
/** A platform-independent 64-bit unsigned integer type. */
typedef unsigned long long uint64;

template <typename Type>
inline void deleteAndZero(Type &pointer) {
  delete pointer;
  pointer = NULL;
}
#define EMPTY_STR_PTR(ptr) (ptr == NULL || ptr[0] == '\0')

//<!************************************************************************
class UtilAll {
 public:
  static bool startsWith_retry(const string &topic);
  static string getRetryTopic(const string &consumerGroup);

  static void Trim(string &str);
  static bool isBlank(const string &str);
  static uint64 hexstr2ull(const char *str);
  static int64 str2ll(const char *str);
  static string bytes2string(const char *bytes, int len);

  template <typename T>
  static string to_string(const T &n) {
    std::ostringstream stm;
    stm << n;
    return stm.str();
  }

  static bool to_bool(std::string const &s) { return atoi(s.c_str()); }

  static bool SplitURL(const string &serverURL, string &addr, short &nPort);
  static int Split(vector<string> &ret_, const string &strIn, const char sep);
  static int Split(vector<string> &ret_, const string &strIn,
                   const string &sep);

  static int32_t StringToInt32(const std::string &str, int32_t &out);
  static int64_t StringToInt64(const std::string &str, int64_t &val);

  static string getLocalHostName();
  static string getLocalAddress();
  static string getHomeDirectory();

  static int getRandomNum(int baseNum);

  static string getProcessName();

  static uint64_t currentTimeMillis();
  static uint64_t currentTimeSeconds();

  static bool deflate(std::string &input, std::string &out, int level);
  static bool inflate(std::string &input, std::string &out);

 private:
  static std::string s_localHostName;
  static std::string s_localIpAddress;
};
//<!***************************************************************************
}  //<!end namespace;
#endif