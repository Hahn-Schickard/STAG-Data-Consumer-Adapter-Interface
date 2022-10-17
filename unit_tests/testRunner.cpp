#include "HaSLL/LoggerManager.hpp"
#include "HaSLL/SPD_LoggerRepository.hpp"
#include "gtest/gtest.h"

int main(int argc, char** argv) {
  auto config = HaSLL::SPD_Configuration("./log", "logfile.log",
      "[%Y-%m-%d-%H:%M:%S:%F %z][%n]%^[%l]: %v%$", HaSLL::SeverityLevel::TRACE,
      true, 8192, 2, 25, 100, 1);
  auto repo = std::make_shared<HaSLL::SPD_LoggerRepository>(config);
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
