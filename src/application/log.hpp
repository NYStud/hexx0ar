#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <SDL2/SDL_timer.h>

#include "application/version.hpp"

static std::time_t time_now = std::time(nullptr);

//some color macros
#ifndef WIN64

#define TXTBLK std::string("\e[0;30m")
#define TXTRED std::string("\e[0;31m")
#define TXTGRN std::string("\e[0;32m")
#define TXTYLW std::string("\e[0;33m")
#define TXTBLU std::string("\e[0;34m")
#define TXTPUR std::string("\e[0;35m")
#define TXTCYN std::string("\e[0;36m")
#define TXTWHT std::string("\e[0;37m")
#define BLDBLK std::string("\e[1;30m")
#define BLDRED std::string("\e[1;31m")
#define BLDGRN std::string("\e[1;32m")
#define BLDYLW std::string("\e[1;33m")
#define BLDBLU std::string("\e[1;34m")
#define BLDPUR std::string("\e[1;35m")
#define BLDCYN std::string("\e[1;36m")
#define BLDWHT std::string("\e[1;37m")
#define UNKBLK std::string("\e[4;30m")
#define UNDRED std::string("\e[4;31m")
#define UNDGRN std::string("\e[4;32m")
#define UNDYLW std::string("\e[4;33m")
#define UNDBLU std::string("\e[4;34m")
#define UNDPUR std::string("\e[4;35m")
#define UNDCYN std::string("\e[4;36m")
#define UNDWHT std::string("\e[4;37m")
#define BAKBLK std::string("\e[40m")
#define BAKRED std::string("\e[41m")
#define BAKGRN std::string("\e[42m")
#define BAKYLW std::string("\e[43m")
#define BAKBLU std::string("\e[44m")
#define BAKPUR std::string("\e[45m")
#define BAKCYN std::string("\e[46m")
#define BAKWHT std::string("\e[47m")
#define TXTRST std::string("\e[0m")

// color macros (yes, they may not have parantheses around msg)

#define COLOR_BLK(msg) TXTBLK + msg + TXTRST
#define COLOR_RED(msg) TXTRED + msg + TXTRST
#define COLOR_GRN(msg) TXTGRN + msg + TXTRST
#define COLOR_YLW(msg) TXTYLW + msg + TXTRST
#define COLOR_BLU(msg) TXTBLU + msg + TXTRST
#define COLOR_PUR(msg) TXTPUR + msg + TXTRST
#define COLOR_CYN(msg) TXTCYN + msg + TXTRST
#define COLOR_WHT(msg) TXTWHT + msg + TXTRST
#define COLOR_BLDBLK(msg) BLDBLK + msg + TXTRST
#define COLOR_BLDRED(msg) BLDRED + msg + TXTRST
#define COLOR_BLDGRN(msg) BLDGRN + msg + TXTRST
#define COLOR_BLDYLW(msg) BLDYLW + msg + TXTRST
#define COLOR_BLDBLU(msg) BLDBLU + msg + TXTRST
#define COLOR_BLDPUR(msg) BLDPUR + msg + TXTRST
#define COLOR_BLDCYN(msg) BLDCYN + msg + TXTRST
#define COLOR_BLDWHT(msg) BLDWHT + msg + TXTRST
#define COLOR_UNKBLK(msg) UNKBLK + msg + TXTRST
#define COLOR_UNDRED(msg) UNDRED + msg + TXTRST
#define COLOR_UNDGRN(msg) UNDGRN + msg + TXTRST
#define COLOR_UNDYLW(msg) UNDYLW + msg + TXTRST
#define COLOR_UNDBLU(msg) UNDBLU + msg + TXTRST
#define COLOR_UNDPUR(msg) UNDPUR + msg + TXTRST
#define COLOR_UNDCYN(msg) UNDCYN + msg + TXTRST
#define COLOR_UNDWHT(msg) UNDWHT + msg + TXTRST
#define COLOR_BAKBLK(msg) BAKBLK + msg + TXTRST
#define COLOR_BAKRED(msg) BAKRED + msg + TXTRST
#define COLOR_BAKGRN(msg) BAKGRN + msg + TXTRST
#define COLOR_BAKYLW(msg) BAKYLW + msg + TXTRST
#define COLOR_BAKBLU(msg) BAKBLU + msg + TXTRST
#define COLOR_BAKPUR(msg) BAKPUR + msg + TXTRST
#define COLOR_BAKCYN(msg) BAKCYN + msg + TXTRST
#define COLOR_BAKWHT(msg) BAKWHT + msg + TXTRST

#else

#define TXTBLK std::string()
#define TXTRED std::string()
#define TXTGRN std::string()
#define TXTYLW std::string()
#define TXTBLU std::string()
#define TXTPUR std::string()
#define TXTCYN std::string()
#define TXTWHT std::string()
#define BLDBLK std::string()
#define BLDRED std::string()
#define BLDGRN std::string()
#define BLDYLW std::string()
#define BLDBLU std::string()
#define BLDPUR std::string()
#define BLDCYN std::string()
#define BLDWHT std::string()
#define UNKBLK std::string()
#define UNDRED std::string()
#define UNDGRN std::string()
#define UNDYLW std::string()
#define UNDBLU std::string()
#define UNDPUR std::string()
#define UNDCYN std::string()
#define UNDWHT std::string()
#define BAKBLK std::string()
#define BAKRED std::string()
#define BAKGRN std::string()
#define BAKYLW std::string()
#define BAKBLU std::string()
#define BAKPUR std::string()
#define BAKCYN std::string()
#define BAKWHT std::string()
#define TXTRST std::string()

#define COLOR_BLK(msg) msg
#define COLOR_RED(msg) msg
#define COLOR_GRN(msg) msg
#define COLOR_YLW(msg) msg
#define COLOR_BLU(msg) msg
#define COLOR_PUR(msg) msg
#define COLOR_CYN(msg) msg
#define COLOR_WHT(msg) msg
#define COLOR_BLDBLK(msg) msg
#define COLOR_BLDRED(msg) msg
#define COLOR_BLDGRN(msg) msg
#define COLOR_BLDYLW(msg) msg
#define COLOR_BLDBLU(msg) msg
#define COLOR_BLDPUR(msg) msg
#define COLOR_BLDCYN(msg) msg
#define COLOR_BLDWHT(msg) msg
#define COLOR_UNKBLK(msg) msg
#define COLOR_UNDRED(msg) msg
#define COLOR_UNDGRN(msg) msg
#define COLOR_UNDYLW(msg) msg
#define COLOR_UNDBLU(msg) msg
#define COLOR_UNDPUR(msg) msg
#define COLOR_UNDCYN(msg) msg
#define COLOR_UNDWHT(msg) msg
#define COLOR_BAKBLK(msg) msg
#define COLOR_BAKRED(msg) msg
#define COLOR_BAKGRN(msg) msg
#define COLOR_BAKYLW(msg) msg
#define COLOR_BAKBLU(msg) msg
#define COLOR_BAKPUR(msg) msg
#define COLOR_BAKCYN(msg) msg
#define COLOR_BAKWHT(msg) msg

#endif

//these macros provide the logging methods

#define LOG_SHORT(msg) Logger::get()._log(msg);
#define LOG_SHORT_ERROR(msg) Logger::get()._log_error(COLOR_RED(msg));
#define LOG_SHORT_INFO(msg) LOG_SHORT(COLOR_GRN(msg))
#define LOG_SHORT_WARN(msg) LOG_SHORT(COLOR_YLW(msg))

#define LOG_MSG(msg) LOG_SHORT(std::string(__FILE__) + ":" + TXTCYN + std::string(__FUNCTION__) + "():" + std::to_string(__LINE__) + TXTRST + " " + msg)
#define LOG_ERROR(msg) Logger::get()._log_error(std::string(__FILE__) + ":" + TXTCYN + std::string(__FUNCTION__) + "():" + std::to_string(__LINE__) + TXTRST + " " + COLOR_RED(msg));
#define LOG_INFO(msg) LOG_MSG(COLOR_GRN(msg))
#define LOG_WARN(msg) LOG_MSG(COLOR_YLW(msg))

#define LOG(msg) LOG_INFO(msg)

#ifdef DEBUG
#define LOG_SHORT_DEBUG(msg) LOG_SHORT(TXTBLU + msg + TXTRST)
#define LOG_DEBUG(msg) LOG_MSG(TXTBLU + msg + TXTRST)
#else
#define LOG_SHORT_DEBUG(msg)
#define LOG_DEBUG(msg)
#endif

class Logger {
private:
  Logger() {
    _log("starting bluebuild version " + std::to_string(BLUEBUILD_VERSION) + " build on " + std::string(__DATE__) + " " + std::string(__TIME__) + " using " + std::string(__VERSION__));
  };

  std::ofstream m_log_file;

  Logger(Logger const &);

  void operator=(Logger const &);

public:
  static Logger &get() {
    static Logger logger;
    return logger;
  }

  void _log_file(std::string msg);
  void _log(std::string msg);
  void _log_error(std::string msg);

  void setLogPath(std::string path);
};
