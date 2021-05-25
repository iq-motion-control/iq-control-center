#ifndef SCHMI_ERROR_HANDLER_STD_HPPP
#define SCHMI_ERROR_HANDLER_STD_HPPP

#include <QString>

#include "Schmi/error_handler_interface.hpp"
#include "main.h"

#include <iostream>
#include <sstream>

namespace Schmi {

class ErrorHandlerStd : public ErrorHandlerInterface {
 public:
  ErrorHandlerStd(){};
  ~ErrorHandlerStd(){};

  void Init(const Schmi::Error& error) override;
  void Display() override;
  void DisplayAndDie() override;

 private:
  Error error_;
};
}  // namespace Schmi

#endif  // SCHMI_ERROR_HANDLER_STD_HPPP
