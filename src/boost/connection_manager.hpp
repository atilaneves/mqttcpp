#ifndef CONNECTION_MANAGER_H_
#define CONNECTION_MANAGER_H_

#include <set>
#include "connection.hpp"


class ConnectionManager {
public:

  ConnectionManager(const ConnectionManager&) = delete;
  ConnectionManager& operator=(const ConnectionManager&) = delete;
  ConnectionManager(ConnectionManager&&) = delete;
  ConnectionManager& operator=(ConnectionManager&&) = delete;

  ConnectionManager();

  void start(ConnectionPtr c);
  void stop(ConnectionPtr c);
  void stopAll();

private:

  std::set<ConnectionPtr> _connections;
};


#endif // CONNECTIONMANAGER_H_
