#ifndef SCHMI_ERROR_HANDLER_HPP
#define SCHMI_ERROR_HANDLER_HPP

namespace Schmi {

struct Error {
  //limitied the error string sizes so no malloc need and no big errors;
  char error_location[256];
  char error_string[256];
  int err_num;
};

class ErrorHandlerInterface {
 public:
  virtual void Init(const Schmi::Error& error) = 0;
  virtual void Display() = 0;
  virtual void DisplayAndDie() = 0;
};

}  // namespace  Schmi

#endif  // SCHMI_ERROR_HANDLER_HPP
