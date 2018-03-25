#include "log.hpp"

void Logger::_log_file(std::string msg) {
  if (m_log_file.is_open()) {
    m_log_file << std::put_time(std::localtime(&time_now), "%OH%OM") << ":" << SDL_GetTicks() << " " << msg
               << std::endl;
  }
}

void Logger::_log(std::string msg) {
  std::cout << SDL_GetTicks() << " " << msg << std::endl;
  _log_file(msg);
}

void Logger::_log_error(std::string msg) {
  std::cerr << SDL_GetTicks() << " " << msg << std::endl;
  _log_file(msg);
}

void Logger::setLogPath(std::string path) {
  m_log_file.open(path);
  _log_file(
          "starting bluebuild version " + std::to_string(BLUEBUILD_VERSION) + " build on " + std::string(__DATE__) + " " +
          std::string(__TIME__) + " using " + std::string(__VERSION__));
}