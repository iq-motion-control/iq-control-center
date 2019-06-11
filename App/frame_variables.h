/*
    Copyright 2018 - 2019 IQ Motion Control   	dskart11@gmail.com

    This file is part of IQ Control Center.

    IQ Control Center is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    IQ Control Center is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef FRAME_VARIABLES_H
#define FRAME_VARIABLES_H

#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include <QString>

#include <string>
#include <map>
#include "IQ_api/json_cpp.hpp"


class FrameVariables
{
  public:
    FrameVariables(){}
    uint8_t frame_type_ = 0;
    // 0 = error
    // 1 = combobox
    // 2 = spinbox
    // 3 = switch
    // 4 = testing
    // 5 = button

    struct ComboFrame
    {
       std::vector<std::string> list_names;
       std::vector<uint8_t>     list_values;
       std::string              info;
    };

    struct SpinFrame
    {
       double maximum;
       double minimum;
       double single_step;
       double decimal;
       std::string unit;
       bool nan;
       std::string info;
    };

    struct SwitchFrame
    {
    };

    struct TestingFrame
    {
        double maximum;
        double minimum;
        double single_step;
        double decimal;
        std::string unit;
    };

    struct ButtonFrame
    {
      std::string info;
    };

    ComboFrame   combo_frame_;
    SpinFrame    spin_frame_;
    SwitchFrame  switch_frame_;
    TestingFrame testing_frame_;
    ButtonFrame  button_frame_;

};

std::map<std::string, FrameVariables*> FrameVariablesFromJson(const std::string &file_name, const std::string &folder_path);
std::map<std::string, FrameVariables*> CreateFrameVariablesMap(const Json::Value &custom_client);
FrameVariables* CreateFrameVariables(const Json::Value &param);

#endif // FRAME_VARIABLES_H