#include "Data_Consumer_Adapter_Interface/DataConsumerAdapterInterface.hpp"

#include "Event_Model/AsyncEventSource.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "Variant_Visitor.hpp"

#include <exception>
#include <iostream>
#include <set>

using namespace std;
using namespace Data_Consumer_Adapter;
using namespace HaSLL;
using namespace Information_Model;
using namespace Event_Model;

void printException(const exception& e, int level = 0) {
  cerr << string(level, ' ') << "Exception: " << e.what() << endl;
  try {
    rethrow_if_nested(e);
  } catch (const exception& nested_exception) {
    printException(nested_exception, level + 1);
  } catch (...) {
  }
}

int main() {
  auto status = EXIT_SUCCESS;
  try {
    LoggerManager::initialise(makeDefaultRepository());

    try {
      auto event_source =
          std::make_shared<AsyncEventSource<ModelRepositoryEvent>>(
              [](exception_ptr ex_ptr) {
                try {
                  if (ex_ptr) {
                    rethrow_exception(ex_ptr);
                  }
                } catch (const exception& ex) {
                  printException(ex);
                }
              });
      auto dcai = std::make_shared<DataConsumerAdapterInterface>(
          event_source, "example");

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
