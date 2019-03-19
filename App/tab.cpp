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

#include "tab.h"

Tab::Tab(QWidget *parent, uint8_t obj_idn, std::vector<std::string> client_file) :
  QWidget(parent),
  parent_(parent),
  clients_folder_path_(client_file[0]),
  client_file_name_(client_file[1]),
  client_map_(ClientsFromJson(obj_idn, client_file_name_, clients_folder_path_)),
  frame_variables_map_(FrameVariablesFromJson(client_file_name_, clients_folder_path_))
{

}

void Tab::CreateFrames()
{
  gridLayout_ = new QGridLayout(parent_);
  gridLayout_->setSpacing(6);
  gridLayout_->setContentsMargins(11, 11, 11, 11);
  gridLayout_->setObjectName(QStringLiteral("gridLayout"));

  int frame_vertical_position = 0;
  for(std::pair<std::string, Client*> client: client_map_)
  {
    for(std::pair<std::string, ClientEntryAbstract*> client_entry: client.second->client_entry_map_)
    {
      std::string client_entry_descriptor = client_entry.first;
      FrameVariables* fv =  frame_variables_map_[client_entry_descriptor];
      uint8_t frame_type = fv->frame_type_;

      switch(frame_type)
      {
        case 1:
        {
          FrameCombo *fc = new FrameCombo(parent_, client.second, client_entry, fv);
          gridLayout_->addWidget(fc,frame_vertical_position, 1, 1, 1);
          ConnectFrameCombo(fc);
          frame_map_[client_entry_descriptor] = fc;
          break;
        }
        case 2:
        {
          FrameSpinBox *fsb = new FrameSpinBox(parent_, client.second, client_entry, fv);
          gridLayout_->addWidget(fsb,frame_vertical_position, 1, 1, 1);
          ConnectFrameSpinBox(fsb);
          frame_map_[client_entry_descriptor] = fsb;
          break;
        }
        case 3:
        {
          FrameSwitch *fs = new FrameSwitch(parent_, client.second, client_entry);
          gridLayout_->addWidget(fs->frame_,frame_vertical_position, 1, 1, 1);
          ConnectFrameSwitch(fs);
//          frame_map_[client_entry_descriptor] = fs;
          break;
        }
        case 4:
        {
          FrameTesting *ft = new FrameTesting(parent_, client.second, client_entry, fv);
          gridLayout_->addWidget(ft,frame_vertical_position, 1, 1, 1);
          ConnectFrameTesting(ft);
          frame_map_[client_entry_descriptor] = ft;
          break;
        }
      }
      ++frame_vertical_position;
    }
  }

  QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
  gridLayout_->addItem(verticalSpacer, frame_vertical_position, 1, 1, 1);
}

void Tab::CheckSavedValues()
{
  for(std::pair<std::string, Frame*> frame: frame_map_)
  {
    int frame_type = frame.second->frame_type_;
    switch(frame_type)
    {
      case 1:
      {
        FrameCombo *fc = nullptr;
        if(!(fc = dynamic_cast<FrameCombo*>(frame.second)))
           break;
        fc->GetSavedValue();
        break;
      }
      case 2:
      {
        FrameSpinBox *fsb = nullptr;
        if(!(fsb = dynamic_cast<FrameSpinBox*>(frame.second)))
           break;
        fsb->GetSavedValue();
      }
    }
  }
}

void Tab::SaveDefaults(std::map<std::string,double> default_value_map)
{
  for(std::pair<std::string, double> default_value: default_value_map)
  {
    Frame *frame = frame_map_[default_value.first];
    int frame_type = frame->frame_type_;
    switch(frame_type)
    {
      case 1:
      {
        FrameCombo *fc = nullptr;
        if(!(fc = dynamic_cast<FrameCombo*>(frame)))
          break;
        fc->value_ = default_value.second;
        fc->SaveValue();
        break;
      }
      case 2:
      {
        FrameSpinBox *fsb = nullptr;
        if(!(fsb = dynamic_cast<FrameSpinBox*>(frame)))
          break;
        fsb->value_ = default_value.second;
        fsb->SaveValue();
      }
    }
  }
}


void Tab::ConnectFrameCombo(FrameCombo *fc)
{
  connect(fc->push_button_save_, SIGNAL(clicked()),fc, SLOT(SaveValue()));
  connect(fc->push_button_default_, SIGNAL(clicked()),fc, SLOT(GetSavedValue()));
  connect(fc->combo_box_, QOverload<int>::of(&QComboBox::currentIndexChanged), fc, &FrameCombo::ComboBoxIndexChange);
}

void Tab::ConnectFrameSpinBox(FrameSpinBox *fsb)
{
  connect(fsb->push_button_save_, SIGNAL(clicked()), fsb, SLOT(SaveValue()));
  connect(fsb->push_button_default_, SIGNAL(clicked()), fsb, SLOT(GetSavedValue()));
  connect(fsb->spin_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), fsb, &FrameSpinBox::SpinBoxValueChanged);
}

void Tab::ConnectFrameSwitch(FrameSwitch *fs)
{
  connect(fs->switch_, SIGNAL(SwitchChanged(bool)), fs, SLOT(SwitchClicked(bool)));
}

void Tab::ConnectFrameTesting(FrameTesting *ft)
{
  connect(ft->spin_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ft, &FrameTesting::SpinBoxValueChanged);
  connect(ft->push_button_set_, SIGNAL(clicked()), ft, SLOT(SetValue()));
}

Tab::~Tab()
{
  ClearWidgets(gridLayout_);
  delete gridLayout_;
}

void ClearWidgets(QGridLayout* layout) {
   if (! layout)
      return;
   while (auto item = layout->takeAt(0)) {
      delete item->widget();
   }
}
