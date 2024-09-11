#pragma once

#include <functional>
#include <memory>

class Buffer;
class Connection;

using ConnectionPtr = std::shared_ptr<Connection>;
using TimerCallback = std::function<void()>;

using CloseCallback = std::function<void(const ConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const ConnectionPtr&)>;

using MessageCallback = std::function<void(const ConnectionPtr&, Buffer*)>;