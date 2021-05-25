#ifndef SCHMI_STD_EXCEPTION_HPP
#define SCHMI_STD_EXCEPTION_HPP

#include <exception>
#include <fstream>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>

#include <iostream>

namespace Schmi {

class StdException : virtual public std::runtime_error {
 public:
  StdException(const std::string& err_msg) : std::runtime_error(err_msg){};
};
}

#endif  // SCHMI_STD_EXCEPTION_HPP