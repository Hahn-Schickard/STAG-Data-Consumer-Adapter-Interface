#ifndef __MULTITHREADING_STOPABLE_TASK_HPP
#define __MULTITHREADING_STOPABLE_TASK_HPP

#include <chrono>
#include <future>

// This header will be moved into its own library, so it could be used by
// Technology Adapters and Data Consumers alike
class Stoppable {
  std::promise<void> exitSignal_;
  std::future<void> exitFuture_;

  virtual void run() = 0;

public:
  Stoppable() : exitFuture_(exitSignal_.get_future()) {}
  Stoppable(Stoppable &&instance)
      : exitSignal_(std::move(instance.exitSignal_)),
        exitFuture_(std::move(instance.exitFuture_)) {}
  Stoppable &operator=(Stoppable &&instance) {
    exitFuture_ = std::move(instance.exitFuture_);
    exitSignal_ = std::move(instance.exitSignal_);
    return *this;
  }

  virtual ~Stoppable() = default;

  void start() { run(); }

  bool stopRequested() {
    return exitFuture_.wait_for(std::chrono::milliseconds(0)) ==
                   std::future_status::timeout
               ? false
               : true;
  }

  void stop() { exitSignal_.set_value(); }
};

#endif //__MULTITHREADING_STOPABLE_TASK_HPP