#include "DataConsumerAdapter.hpp"

#include <Information_Model_Mocks/MockBuilder.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <thread>

namespace Data_Consumer_Adapter::testing {
using namespace std;
using namespace Information_Model;
using namespace Information_Model::testing;
using namespace ::testing;

struct DataConsumerAdapterMock : DataConsumerAdapter {
  DataConsumerAdapterMock(const string& name, const DataConnector& connector)
      : DataConsumerAdapter(name, connector) {
    ON_CALL(*this, start).WillByDefault([this]() {
      DataConsumerAdapter::start();
    });
    ON_CALL(*this, stop).WillByDefault([this]() {
      DataConsumerAdapter::stop();
    });
  }

  MOCK_METHOD(void, registrate, (const DevicePtr&), (override));
  MOCK_METHOD(void, deregistrate, (const string&), (override));
  MOCK_METHOD(void, start, (), (override));
  MOCK_METHOD(void, stop, (), (override));
};

using DataConsumerAdapterMockPtr = shared_ptr<DataConsumerAdapterMock>;

struct FakeSource {
  DataConnectionPtr connect(const DataNotifier& notifier) {
    auto connection = make_shared<FakeConnection>(notifier);
    connections_.emplace_back(connection);
    return connection;
  }

  void notify(const RegistryChangePtr& event) {
    for (auto it = connections_.begin(); it != connections_.end();) {
      if (auto connection = it->lock()) {
        connection->call(event);
        ++it;
      } else {
        it = connections_.erase(it);
      }
    }
  }

private:
  struct FakeConnection : DataConnection {
    FakeConnection(const DataNotifier& notifier) : notify_(notifier) {}

    void call(const RegistryChangePtr& event) { notify_(event); }

  private:
    DataNotifier notify_;
  };

  using WeakConnectionPtr = weak_ptr<FakeConnection>;
  vector<WeakConnectionPtr> connections_;
};

using FakeSourcePtr = shared_ptr<FakeSource>;

struct DataConsumerAdapterTests : public ::testing::Test {
  void SetUp() override {
    source = make_shared<FakeSource>();
    auto connector = [weak_source = weak_ptr<FakeSource>(source)](
                         const DataNotifier& notifier) {
      if (auto source = weak_source.lock()) {
        return source->connect(notifier);
      } else {
        return DataConnectionPtr();
      }
    };
    mock = make_shared<DataConsumerAdapterMock>("Mock adapter", connector);
    tested = mock;
  }

  void TearDown() override {
    tested.reset();
    mock.reset();
    source.reset();
  }

  FakeSourcePtr source;
  DataConsumerAdapterMockPtr mock;
  DataConsumerAdapterPtr tested;
};

DevicePtr makeDevice(const string& id) {
  auto builder = make_unique<MockBuilder>();
  builder->setDeviceInfo(id, BuildInfo{"Mock", "Mock device"});
  builder->addReadable(
      BuildInfo{"readable", "readable metric mock"}, DataType::Boolean);
  return builder->result();
}

TEST_F(DataConsumerAdapterTests, returnsName) {
  EXPECT_EQ(tested->name(), "Mock adapter");
}

TEST_F(DataConsumerAdapterTests, canStart) {
  EXPECT_CALL(*mock, start()).Times(1);

  EXPECT_NO_THROW(tested->start());
}

TEST_F(DataConsumerAdapterTests, canStop) {
  EXPECT_CALL(*mock, stop()).Times(1);

  EXPECT_NO_THROW(tested->stop());
}

TEST_F(DataConsumerAdapterTests, canRegister) {
  auto device = makeDevice("12345");
  EXPECT_CALL(*mock, registrate(device)).Times(1);

  EXPECT_NO_THROW(source->notify(make_shared<RegistryChange>(device)));
}

TEST_F(DataConsumerAdapterTests, canHandleRegisterException) {
  auto expected = runtime_error("Test exception throwing during registration");
  auto device = makeDevice("01");
  EXPECT_CALL(*mock, registrate(device)).Times(1).WillOnce(Throw(expected));

  EXPECT_NO_THROW(source->notify(make_shared<RegistryChange>(device)));
}

constexpr size_t INITIAL_MODEL_SIZE = 5;

TEST_F(DataConsumerAdapterTests, canInit) {
  vector<DevicePtr> devices;
  devices.reserve(INITIAL_MODEL_SIZE);
  for (size_t i = 0; i < INITIAL_MODEL_SIZE; ++i) {
    devices.emplace_back(makeDevice(to_string(i)));
  }

  EXPECT_FALSE(devices.empty());
  EXPECT_EQ(INITIAL_MODEL_SIZE, devices.size());

  for (const auto& device : devices) {
    EXPECT_CALL(*mock, registrate(device)).Times(1);
  }

  auto init_model_thread = thread(
      [&devices, weak_adapter = weak_ptr<DataConsumerAdapter>(tested)]() {
        if (auto adapter = weak_adapter.lock()) {
          adapter->initialiseModel(devices);
        }
      });
  init_model_thread.join(); // block until the task is finished
  EXPECT_FALSE(init_model_thread.joinable());
}

TEST_F(DataConsumerAdapterTests, canDeregister) {
  string id = "12345";
  EXPECT_CALL(*mock, deregistrate(id)).Times(1);

  auto event = make_shared<RegistryChange>(id);
  source->notify(event);
}

TEST_F(DataConsumerAdapterTests, canHandleDeregisterException) {
  auto expected =
      runtime_error("Test exception throwing during deregistration");
  EXPECT_CALL(*mock, deregistrate("")).Times(1).WillOnce(Throw(expected));

  EXPECT_NO_THROW(source->notify(make_shared<RegistryChange>("")));
}

TEST_F(DataConsumerAdapterTests, canNotDeregisterDuringInit) {
  string id = "0";
  auto device1 = makeDevice(id);
  auto device2 = makeDevice("1");

  {
    InSequence sequence;
    EXPECT_CALL(*mock, registrate(device1)).Times(1).WillOnce([]() {
      this_thread::sleep_for(100ms);
    });
    EXPECT_CALL(*mock, registrate(device2)).Times(1).WillOnce([]() {
      this_thread::sleep_for(100ms);
    });
    EXPECT_CALL(*mock, deregistrate(id)).Times(1);
    EXPECT_CALL(*mock, registrate(_)).Times(0);
    EXPECT_CALL(*mock, deregistrate(_)).Times(0);
  }

  auto start = chrono::system_clock::now();
  tested->initialiseModel({device1, device2});
  auto stop = chrono::system_clock::now();

  EXPECT_NO_THROW(source->notify(make_shared<RegistryChange>(id)));

  auto duration = stop - start;
  EXPECT_THAT(duration, AllOf(Gt(200ms), Lt(300ms)));
}
} // namespace Data_Consumer_Adapter::testing