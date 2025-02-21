#include "DataConsumerAdapterInterface.hpp"
#include "Event_Model/AsyncEventSource.hpp"

#include "HaSLL/LoggerManager.hpp"
#include "Information_Model/mocks/DeviceMockBuilder.hpp"
#include "Variant_Visitor.hpp"

#include <exception>
#include <iostream>
#include <set>

using namespace std;
using namespace Data_Consumer_Adapter;
using namespace HaSLL;
using namespace Information_Model;

// NOLINTNEXTLINE(readability-identifier-naming)
struct DCAI_Example : DataConsumerAdapterInterface {
  explicit DCAI_Example(const ModelEventSourcePtr& source)
      /* Never move into Model Event Source ptr either! */
      : DataConsumerAdapterInterface(source, "Example DCAI") {} // NOLINT

  void start(const Devices& devices = {}) final {
    DataConsumerAdapterInterface::start(devices);
  }

private:
  void registrate(const Information_Model::NonemptyDevicePtr& device) override {
    auto [_, emplaced] = devices_.emplace(device->getElementId());
    if (emplaced) {
      logger->trace("Device: {} was registered!", device->getElementName());
    } else {
      logger->trace("Device: {} was already registered. Ignoring new instance!",
          device->getElementName());
    }
  }

  void deregistrate(const string& device_id) override {
    auto it = devices_.find(device_id);
    if (it != devices_.end()) {
      logger->trace("Device: {} was deregistered!", device_id);
    } else {
      string error_msg = "Device " + device_id + " does not exist!";
      throw runtime_error(error_msg);
    }
  }

  set<string> devices_;
};

void printException(const exception& e, int level = 0) {
  cerr << string(level, ' ') << "Exception: " << e.what() << endl;
  try {
    rethrow_if_nested(e);
  } catch (const exception& nested_exception) {
    printException(nested_exception, level + 1);
  } catch (...) {
    cerr << "Caught an unhandled exception" << endl;
  }
}

class EventSourceFake
    : public Event_Model::AsyncEventSource<ModelRepositoryEvent> {

  LoggerPtr logger_;

  void handleException(const exception_ptr& ex_ptr) const {
    try {
      if (ex_ptr) {
        rethrow_exception(ex_ptr);
      }
    } catch (const exception& ex) {
      logger_->error(
          "An exception occurred while notifying a listener: {}", ex.what());
      printException(ex);
    }
  }

public:
  EventSourceFake()
      : AsyncEventSource(
            bind(&EventSourceFake::handleException, this, placeholders::_1)),
        logger_(LoggerManager::registerTypedLogger(this)) {}

  void registerDevice(const NonemptyDevicePtr& device) {
    auto event = std::make_shared<ModelRepositoryEvent>(device);
    logger_->trace(
        "Notifing listeners that Device {} is available for registration.",
        device->getElementId());
    notify(move(event));
  }

  void deregisterDevice(const string& identifier) {
    auto event = std::make_shared<ModelRepositoryEvent>(identifier);
    logger_->trace(
        "Notifing listeners that Device {} is no longer available", identifier);
    notify(move(event));
  }
};

int main() {
  auto status = EXIT_SUCCESS;
  try {
    LoggerManager::initialise(makeDefaultRepository());
    try {

      auto event_source = make_shared<EventSourceFake>();
      auto dcai = DCAI_Example(event_source);

      dcai.start();

      string device_id = "1234";
      {
        auto builder =
            make_unique<Information_Model::testing::DeviceMockBuilder>();
        builder->buildDeviceBase(
            device_id, "Mocky", "A mocked device with no elements");
        NonemptyDevicePtr result(builder->getResult());
        event_source->registerDevice(result);
        event_source->registerDevice(
            result); // check if double registration is handled
        builder.reset();
      }

      event_source->deregisterDevice(device_id);
      event_source->deregisterDevice("nonsense ID"); // check bad ID deregister

      this_thread::sleep_for(2s);

      dcai.stop();

      status = EXIT_SUCCESS;
    } catch (const exception& ex) {
      printException(ex);
      status = EXIT_FAILURE;
    }
    LoggerManager::terminate();
  } catch (...) {
    cerr << "Unknown error occurred during program execution." << endl;
    status = EXIT_FAILURE;
  }

  exit(status);
}
