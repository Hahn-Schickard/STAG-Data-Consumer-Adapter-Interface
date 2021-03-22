#include "Data_Consumer_Adapter_Interface/DataConsumerAdapterInterface.hpp"
#include "Event_Model.hpp"
#include "Information_Model/mocks/DeviceMockBuilder.hpp"
#include "LoggerRepository.hpp"
#include "Variant_Visitor.hpp"

#include <iostream>

using namespace std;
using namespace DCAI;
using namespace HaSLL;
using namespace Information_Model;

struct DCAI_Example : DataConsumerAdapterInterface {
  DCAI_Example(ModelEventSourcePtr source)
      : DataConsumerAdapterInterface(source, "Example DCAI") {}

  void start() override final {
    cout << getAdapterName() << " Started!" << endl;
    DataConsumerAdapterInterface::start();
  }

  void stop() override final {
    cout << getAdapterName() << " Received a stop command!" << endl;
    DataConsumerAdapterInterface::stop();
  }

private:
  void handleEvent(shared_ptr<ModelRegistryEvent> event) {
    this->logger_->log(SeverityLevel::TRACE, "Received an event!");
    match(*event.get(),
          [&](string identifier) {
            cout << "Device: " << identifier << " was deregistered!" << endl;
          },
          [&](DevicePtr device) {
            cout << "Device: " << device->getElementName() << ":"
                 << device->getElementId() << " was registered!" << endl;
          });
  }
};

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
public:
  void registerDevice(DevicePtr device) {
    auto event = make_shared<ModelRegistryEvent>(device);
    notify(move(event));
  }

  void deregisterDevice(string identifier) {
    auto event = make_shared<ModelRegistryEvent>(identifier);
    notify(move(event));
  }
};

int main() {
  auto config = HaSLL::Configuration(
      "./log", "logfile.log", "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
      HaSLL::SeverityLevel::TRACE, false, 8192, 2, 25, 100, 1);
  HaSLL::LoggerRepository::initialise(config);

  auto event_source = make_shared<EventSourceFake>();
  auto dcai = DCAI_Example(event_source);

  dcai.start();

  string device_id = "1234";
  {
    auto builder = new Information_Model::testing::DeviceMockBuilder();
    builder->buildDeviceBase(device_id, "Mocky",
                             "A mocked device with no elements");
    event_source->registerDevice(builder->getResult());
    delete builder;
  }

  event_source->deregisterDevice(device_id);

  dcai.stop();

  return EXIT_SUCCESS;
}
