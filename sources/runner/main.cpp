#include "DataConsumerAdapterInterface.hpp"
#include "Event_Model/AsyncEventSource.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"
#include "Information_Model/mocks/DeviceMockBuilder.hpp"
#include "Variant_Visitor.hpp"

#include <set>

using namespace std;
using namespace DCAI;
using namespace HaSLL;
using namespace Information_Model;

struct DCAI_Example : DataConsumerAdapterInterface {
  DCAI_Example(ModelEventSourcePtr source)
      : DataConsumerAdapterInterface(source, "Example DCAI") {}

  void start() override final {
    this->logger_->log(SeverityLevel::TRACE, "{} Started!", getAdapterName());
    DataConsumerAdapterInterface::start();
  }

  void stop() override final {
    this->logger_->log(
        SeverityLevel::TRACE, "{} Received a stop command!", getAdapterName());
    DataConsumerAdapterInterface::stop();
  }

private:
  void handleEvent(shared_ptr<ModelRegistryEvent> event) {
    this->logger_->log(SeverityLevel::TRACE, "Received an event!");
    match(
        *event.get(),
        [&](string identifier) {
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

class EventSourceFake
    : public Event_Model::AsyncEventSource<ModelRegistryEvent> {

  LoggerPtr logger_;

  void handleException(std::exception_ptr ex_ptr) {
    try {
      if (ex_ptr)
        rethrow_exception(ex_ptr);
    } catch (exception& ex) {
      logger_->log(SeverityLevel::ERROR,
          "An exception occurred while notifing a listener: {}", ex.what());
    }
  }

public:
  EventSourceFake()
      : AsyncEventSource(
            bind(&EventSourceFake::handleException, this, placeholders::_1)),
        logger_(LoggerManager::registerTypedLogger(this)) {}

  void registerDevice(NonemptyDevicePtr device) {
    auto event = std::make_shared<ModelRegistryEvent>(device);
    logger_->log(SeverityLevel::TRACE,
        "Notifing listeners that Device {} is available for registration.",
        device->getElementId());
    notify(move(event));
  }

  void deregisterDevice(string identifier) {
    auto event = make_shared<ModelRegistryEvent>(identifier);
    logger_->log(SeverityLevel::TRACE,
        "Notifing listeners that Device {} is no longer available", identifier);
    notify(move(event));
  }
};

int main() {
  auto config = HaSLL::SPD_Configuration("./log", "logfile.log",
      "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$", HaSLL::SeverityLevel::TRACE,
      true, 8192, 2, 25, 100, 1);
  auto repo = make_shared<SPD_LoggerRepository>(config);
  LoggerManager::initialise(repo);

  auto event_source = make_shared<EventSourceFake>();
  auto dcai = DCAI_Example(event_source);

  dcai.start();

  string device_id = "1234";
  {
    auto builder = new Information_Model::testing::DeviceMockBuilder();
    builder->buildDeviceBase(
        device_id, "Mocky", "A mocked device with no elements");
    NonemptyDevicePtr result(builder->getResult());
    event_source->registerDevice(result);
    event_source->registerDevice(
        result); // check if double registration is handled
    delete builder;
  }

  event_source->deregisterDevice(device_id);
  event_source->deregisterDevice("noncense ID"); // check bad ID deregister

  this_thread::sleep_for(2s);

  dcai.stop();

  return EXIT_SUCCESS;
}
