#include <Data_Consumer_Adapter_Interface/DataConsumerAdapter.hpp>
#include <HaSLL/LoggerManager.hpp>

#include <iostream>

using namespace std;

int main() {
  Information_Model::DevicePtr device;
  Data_Consumer_Adapter::RegistryChangePtr event;
  Data_Consumer_Adapter::DataConnectionPtr connection;
  Data_Consumer_Adapter::DataNotifier notifier;
  Data_Consumer_Adapter::DataConnector connector;
  Data_Consumer_Adapter::DataConsumerAdapterPtr adapter;
  HaSLL::LoggerPtr logger;

  cout << "Integration Test successful" << endl;

  return 0;
}
