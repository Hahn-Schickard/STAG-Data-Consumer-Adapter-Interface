#include "DataConsumerAdapter.hpp"

#include <HaSLL/LoggerManager.hpp>
#include <Information_Model_Mocks/MockBuilder.hpp>
#include <Variant_Visitor/Visitor.hpp>

#include <exception>
#include <iostream>
#include <set>
#include <thread>

using namespace std;
using namespace Data_Consumer_Adapter;
using namespace HaSLL;
using namespace Information_Model;

struct ExampleAdapter : DataConsumerAdapter {
  explicit ExampleAdapter(const DataConnector& connector)
      : DataConsumerAdapter("Example DCAI", connector) {}

private:
  void registrate(const DevicePtr& device) override {
    auto [_, emplaced] = devices_.emplace(device->id());
    if (emplaced) {
      logger->trace("Device: {} was registered!", device->name());
    } else {
      logger->trace("Device: {} was already registered. Ignoring new instance!",
          device->name());
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

struct FakeSource {
  DataConnectionPtr connect(const DataNotifier& notifier) {
    auto connection = make_shared<FakeConnection>(notifier);
    connection_ = connection;
    return connection;
  }

  void notify(const ModelRepositoryEventPtr& event) {
    if (auto locked = connection_.lock()) {
      locked->call(event);
    }
  }

  void registerDevice(const DevicePtr& device) {
    auto event = std::make_shared<ModelRepositoryEvent>(device);
    notify(event);
  }

  void deregisterDevice(const string& identifier) {
    auto event = std::make_shared<ModelRepositoryEvent>(identifier);
    notify(event);
  }

private:
  struct FakeConnection : DataConnection {
    FakeConnection(const DataNotifier& notifier) : notify_(notifier) {}

    void call(const ModelRepositoryEventPtr& event) { notify_(event); }

  private:
    DataNotifier notify_;
  };

  using WeakConnectionPtr = weak_ptr<FakeConnection>;
  WeakConnectionPtr connection_;
};

int main() {
  auto status = EXIT_SUCCESS;
  try {
    LoggerManager::initialise(makeDefaultRepository());
    try {

      auto event_source = FakeSource();
      auto dcai = ExampleAdapter([&event_source](const DataNotifier& notifier) {
        return event_source.connect(notifier);
      });

      dcai.start();

      string device_id = "1234";
      {
        auto builder = make_unique<Information_Model::testing::MockBuilder>();
        builder->setDeviceInfo(
            device_id, BuildInfo{"Mocky", "A mocked device with no elements"});
        builder->addReadable(
            BuildInfo{"readable", "readable metric mock"}, DataType::Boolean);
        DevicePtr result = builder->result();
        event_source.registerDevice(result);
        builder.reset();
      }

      event_source.deregisterDevice(device_id);

      this_thread::sleep_for(2s);

      dcai.stop();

      status = EXIT_SUCCESS;
    } catch (const exception& ex) {
      cerr << "Caught an exception during execution: " << ex.what() << endl;
      status = EXIT_FAILURE;
    }
    LoggerManager::terminate();
  } catch (...) {
    cerr << "Unknown error occurred during program execution." << endl;
    status = EXIT_FAILURE;
  }

  exit(status);
}
