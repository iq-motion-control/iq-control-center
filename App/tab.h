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

#ifndef TAB_H
#define TAB_H


#include <map>

#include "IQ_api/client.hpp"
#include "IQ_api/client_helpers.hpp"

#include "frame_combo.h"
#include "frame_spin_box.h"
#include "frame_switch.h"
#include "frame_testing.h"
#include "frame_button.h"
#include "frame_read_only.h"
#include "frame_variables.h"


class Tab: public QWidget
{
  Q_OBJECT
  public:
    Tab(QWidget *parent, uint8_t obj_idn, std::vector<std::string> client_file);

    ~Tab();

    void CreateFrames();

    void CheckSavedValues();

    void SaveDefaults(std::map<std::string,double> frame_value_map);

    QGridLayout *gridLayout_;

  private:
    void ConnectFrameCombo(FrameCombo *fc);

    void ConnectFrameSpinBox(FrameSpinBox *fsb);

    void ConnectFrameSwitch(FrameSwitch *fs);

    void ConnectFrameTesting(FrameTesting *ft);

    void ConnectFrameButton(FrameButton *fb);

    void ConnectFrameReadOnly(FrameReadOnly *fr);

    std::map<std::string,Frame*> frame_map_;

    QWidget *parent_;
    std::string clients_folder_path_;
    std::string client_file_name_;
    bool using_custom_order_;
    std::map<std::string, std::string> frame_descriptors_;

    std::map<std::string, Client*> client_map_;
    std::map<std::string, FrameVariables*> frame_variables_map_;
};

void ClearWidgets(QGridLayout* layout);

#endif // GENERAL_TAB_H
