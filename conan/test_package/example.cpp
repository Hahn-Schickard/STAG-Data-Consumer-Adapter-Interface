#include "DataConsumerAdapterInterface.hpp"
#include "LoggerRepository.hpp"

#include <iostream>

using namespace std;
using namespace DCAI;

class Fake_DCAI : public DataConsumerAdapterInterface {
  void run() override {
    int i = 0;
    do {
      cout << "Running cycle: " << i << endl;
      i++;
    } while (!stopRequested());
    cout << "Stopped at cycle: " << i << endl;
  }

  void
  handleEvent(shared_ptr<Model_Event_Handler::NotifierEvent> event) override {
    cout << "Received an event! " << endl;
  }

public:
  Fake_DCAI()
      : DataConsumerAdapterInterface("Fake Data Consumer Adapter Interface") {}
};

int main() {
  auto config = HaSLL::Configuration(
      "./log", "logfile.log", "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$",
      HaSLL::SeverityLevel::TRACE, false, 8192, 2, 25, 100, 1);
  HaSLL::LoggerRepository::initialise(config);
  auto adapter = Fake_DCAI();
  cout << adapter.getAdapterName() << endl;
  exit(EXIT_SUCCESS);
}
