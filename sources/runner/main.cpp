#include "DataConsumerAdapterInterface.hpp"
#include "Event_Model/AsyncEventSource.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"
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
  DCAI_Example(ModelEventSourcePtr source)
      /* Never move into Model Event Source ptr either! */
      : DataConsumerAdapterInterface(source, "Example DCAI") {} // NOLINT

  void start(std::vector<Information_Model::DevicePtr> devices = {}) final {
    this->logger_->log(SeverityLevel::TRACE, "{} Started!", name);
    DataConsumerAdapterInterface::start(devices);
  }

private:
  void handleEvent(ModelRegistryEventPtr event) override {
    this->logger_->log(SeverityLevel::TRACE, "Received an event!");
    match(*event,
        [&](const string& identifier) {
          auto it = devices_.find(identifier);
          if (it != devices_.end()) {
            this->logger_->log(SeverityLevel::TRACE,
                "Device: {} was deregistered!", identifier);
          } else {
            string error_msg = "Device " + identifier + " does not exist!";
            throw runtime_error(error_msg);
          }
        },
        [&](NonemptyDevicePtr device) {
          auto emplaced = devices_.emplace(device->getElementId());
          if (emplaced.second) {
            this->logger_->log(SeverityLevel::TRACE,
                "Device: {} was registered!", device->getElementName());
          } else {
            this->logger_->log(SeverityLevel::TRACE,
                "Device: {} was already registered. Ignoring new instance!",
                device->getElementName());
          }
        });
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
  }
}

class EventSourceFake
    : public Event_Model::AsyncEventSource<ModelRepositoryEvent> {

  LoggerPtr logger_;

  void handleException(std::exception_ptr ex_ptr) { // NOLINT
    try {
      if (ex_ptr) {
        rethrow_exception(ex_ptr);
      }
    } catch (const exception& ex) {
      logger_->log(SeverityLevel::ERROR,
          "An exception occurred while notifying a listener: {}", ex.what());
      printException(ex);
    }
  }

public:
  EventSourceFake()
      : AsyncEventSource(
            bind(&EventSourceFake::handleException, this, placeholders::_1)),
        logger_(LoggerManager::registerTypedLogger(this)) {}

  void registerDevice(NonemptyDevicePtr device) {
    auto event = std::make_shared<ModelRepositoryEvent>(device);
    logger_->log(SeverityLevel::TRACE,
        "Notifing listeners that Device {} is available for registration.",
        device->getElementId());
    notify(move(event));
  }

  void deregisterDevice(string identifier) {
    auto event = make_shared<ModelRepositoryEvent>(identifier);
    logger_->log(SeverityLevel::TRACE,
        "Notifing listeners that Device {} is no longer available", identifier);
    notify(move(event));
  }
};

int main() {
  try {
    auto config = HaSLL::SPD_Configuration("./log", "logfile.log",
        "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
        HaSLL::SeverityLevel::TRACE, true, 8192, 2, 25, 100, 1); // NOLINT
    auto repo = make_shared<SPD_LoggerRepository>(config);
    LoggerManager::initialise(repo);

    auto event_source = make_shared<EventSourceFake>();
    auto dcai = DCAI_Example(event_source);

    dcai.start();

    string device_id = "1234";
    {
      auto* builder = new Information_Model::testing::DeviceMockBuilder();
      builder->buildDeviceBase(
          device_id, "Mocky", "A mocked device with no elements");
      NonemptyDevicePtr result(builder->getResult());
      event_source->registerDevice(result);
      event_source->registerDevice(
          result); // check if double registration is handled
      delete builder;
    }

    event_source->deregisterDevice(device_id);
    event_source->deregisterDevice("nonsense ID"); // check bad ID deregister

    this_thread::sleep_for(2s);

    dcai.stop();

    return EXIT_SUCCESS;
  } catch (const exception& ex) {
    printException(ex);
    return EXIT_FAILURE;
  }
}
