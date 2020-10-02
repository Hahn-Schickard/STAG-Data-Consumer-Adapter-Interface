#include "DataConsumerAdapterInterface_MOCK.hpp"
#include "Device_MOCK.hpp"
#include "Notifier.hpp"

#include <gtest/gtest.h>
#include <memory>

using namespace std;
using namespace DCAI::testing;
using namespace Information_Model::testing;
using namespace Model_Event_Handler;

TEST(DCAI_Test, canHandleEvent) {
  shared_ptr<Listener> adapter =
      make_shared<DataConsumerAdapterInterfaceMock>("adopt me!");
  auto notifier = make_unique<Notifier>();
  notifier->registerListener(adapter);

  auto event = make_shared<NotifierEvent>(
      NotifierEventType::NEW_DEVICE_REGISTERED,
      make_shared<MockDevice>("12345", "Mock", "Mock device"));
  EXPECT_CALL(*(static_pointer_cast<DataConsumerAdapterInterfaceMock>(adapter)),
              handleEvent(event));

  notifier->notifyListeners(event);
}

TEST(DCAI_Test, canStart) {
  DataConsumerAdapterInterfaceMock adapter("start me!");
  EXPECT_CALL(adapter, start());

  EXPECT_NO_THROW(adapter.start());
}

TEST(DCAI_Test, canStop) {
  DataConsumerAdapterInterfaceMock adapter("start me!");
  EXPECT_CALL(adapter, stop());

  EXPECT_NO_THROW(adapter.stop());
}