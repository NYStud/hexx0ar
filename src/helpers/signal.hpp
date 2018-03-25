#pragma once

#include <utility>
#include <vector>
#include <functional>

/*
 * simple Signal class implementation, not threadsafe.
 *
 * TODO: disconnect method, maybe if necessary threadsafety for connect & call
 * */
template<typename T>
class Signal;

template <typename res, typename... arg_ts>
class Signal<res(arg_ts...)> {
private:
  std::vector<std::function<res(arg_ts...)>> m_connections;
public:
  void connect(std::function<res(arg_ts...)> conn) {
    m_connections.push_back(conn);
  }
  void operator() (const arg_ts&... args) {
    for(std::function<res(arg_ts...)> conn : m_connections) {
      conn(args...);
    }
  }
};

