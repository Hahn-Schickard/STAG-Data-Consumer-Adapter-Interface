#include "DataConsumerAdapterInterface.hpp"
#include "LoggerRepository.hpp"

#include <iostream>

using namespace std;
using namespace DCAI;

class EventSourceFake : public Event_Model::EventSource<ModelRegistryEvent> {
public:
  void sendEvent(std::shared_ptr<ModelRegistryEvent> event) { notify(event); }
};

class Fake_DCAI : public DataConsumerAdapterInterface {
  void handleEvent(shared_ptr<ModelRegistryEvent> event) override {
    cout << "Received an event! " << endl;
  }

public:
  Fake_DCAI(
      Event_Model::EventSourceInterfacePtr<ModelRegistryEvent> event_source)
      : DataConsumerAdapterInterface("Fake Data Consumer Adapter Interface",
                                     event_source) {}
};

int main() {
  auto config = HaSLL::Configuration(
      "./log", "logfile.log", "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
      HaSLL::SeverityLevel::TRACE, false, 8192, 2, 25, 100, 1);
  HaSLL::LoggerRepository::initialise(config);
  auto adapter = Fake_DCAI(make_shared<EventSourceFake>());
  cout << adapter.getAdapterName() << endl;
  exit(EXIT_SUCCESS);
}
