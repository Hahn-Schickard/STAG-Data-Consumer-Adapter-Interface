#include "Data_Consumer_Adapter_Interface/DataConsumerAdapterInterface.hpp"

#include "Event_Model/AsyncEventSource.hpp"
#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"
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
  try {
    auto repo = make_shared<SPD_LoggerRepository>();
    LoggerManager::initialise(repo);

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
    auto dcai =
        std::make_shared<DataConsumerAdapterInterface>(event_source, "example");

  } catch (const exception& ex) {
    printException(ex);
    exit(EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
}
