/*
    Copyright 2018 - 2019 IQ Motion Control   	raf@iq-control.com

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
#include "mainwindow.h"

Tab::Tab(QWidget *parent, uint8_t obj_idn, std::vector<std::string> client_file) :
  QWidget(parent),
  parent_(parent),
  clients_folder_path_(client_file[0]),
  client_file_name_(client_file[1]),
  using_custom_order_(false),
  frame_descriptors_(),
  client_map_(ClientsFromJson(obj_idn, client_file_name_, clients_folder_path_, &using_custom_order_, &frame_descriptors_)),
  frame_variables_map_(FrameVariablesFromJson(client_file_name_, clients_folder_path_, using_custom_order_))
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

      bool entry_needs_reboot = false;

      //We need to check to see if this is the Module ID entry. if it is,
      //Check to see if the port connection obj_id_ is different from sys_map_ obj_id. if it is
      //Default to making this client's object id = 0 since we know this is an old firmware module

      if(client_entry.second->sub_idn_ == MODULE_ID_SUB_ID && client_entry.second->type_idn_ == MODULE_ID_TYPE_ID){
        entry_needs_reboot = true;

        if(iv.pcon->GetSysMapObjId() != client_entry.second->obj_idn_){
            client_entry.second->obj_idn_ = iv.pcon->GetSysMapObjId();
        }
      }

      FrameVariables* fv =  frame_variables_map_[client_entry_descriptor];

      uint8_t frame_type = fv->frame_type_;

      switch(frame_type)
      {
        case 1:
        {
          FrameCombo *fc = new FrameCombo(parent_, client.second, client_entry, fv, using_custom_order_, QString(frame_descriptors_[client_entry_descriptor].c_str()));
          gridLayout_->addWidget(fc,frame_vertical_position, 1, 1, 1);
          ConnectFrameCombo(fc);
          frame_map_[client_entry_descriptor] = fc;
          break;
        }
        case 2:
        {

          FrameSpinBox *fsb = new FrameSpinBox(parent_, client.second, client_entry, fv, using_custom_order_, QString(frame_descriptors_[client_entry_descriptor].c_str()), entry_needs_reboot);
          gridLayout_->addWidget(fsb,frame_vertical_position, 1, 1, 1);
          ConnectFrameSpinBox(fsb);
          frame_map_[client_entry_descriptor] = fsb;
          break;
        }
        case 3:
        {
          FrameSwitch *fs = new FrameSwitch(parent_, client.second, client_entry, using_custom_order_, QString(frame_descriptors_[client_entry_descriptor].c_str()));
          gridLayout_->addWidget(fs->frame_,frame_vertical_position, 1, 1, 1);
          ConnectFrameSwitch(fs);
//          frame_map_[client_entry_descriptor] = fs;
          break;
        }
        case 4:
        {
          FrameTesting *ft = new FrameTesting(parent_, client.second, client_entry, fv, using_custom_order_, QString(frame_descriptors_[client_entry_descriptor].c_str()));
          gridLayout_->addWidget(ft,frame_vertical_position, 1, 1, 1);
          ConnectFrameTesting(ft);
          frame_map_[client_entry_descriptor] = ft;
          break;
        }
        case 5:
        {
          FrameButton *fb = new FrameButton(parent_, client.second, client_entry, fv, using_custom_order_, QString(frame_descriptors_[client_entry_descriptor].c_str()));
          gridLayout_->addWidget(fb,frame_vertical_position, 1, 1, 1);
          ConnectFrameButton(fb);
          frame_map_[client_entry_descriptor] = fb;
          break;
        }

        case 6:
        {
          FrameReadOnly *fr = new FrameReadOnly(parent_, client.second, client_entry, fv, using_custom_order_, QString(frame_descriptors_[client_entry_descriptor].c_str()));
          gridLayout_->addWidget(fr,frame_vertical_position, 1, 1, 1);
          ConnectFrameReadOnly(fr);
          frame_map_[client_entry_descriptor] = fr;
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
        break;
      }

      case 6:
      {
        FrameReadOnly *fr = nullptr;
        if(!(fr = dynamic_cast<FrameReadOnly *>(frame.second)))
            break;
        fr->GetSavedReadOnlyValue();
        break;
      }
    }
  }
}

bool Tab::IsClose(double val1, double val2, double tolerance){
    return (abs(val1 - val2) <= tolerance);
}

void Tab::SaveDefaults(std::map<std::string,double> default_value_map)
{

    for(std::pair<std::string, double> default_value: default_value_map)
    {
      //If the tab actually has the frame that the defaults file wants, then we can set it.
      //otherwise ignore it altogether
      if(frame_map_.count(default_value.first) > 0){
          Frame *frame = frame_map_[default_value.first];
          int frame_type = frame->frame_type_;
          switch(frame_type)
          {
            case 1:
            {
              FrameCombo *fc = nullptr;
              if(!(fc = dynamic_cast<FrameCombo*>(frame)))
                break;

              //Let's make sure that the value we're trying to save is different than what's on there already
              if(!IsClose(fc->GetFrameValue(), default_value.second)){
                  fc->SetFrameValue(default_value.second);
                  fc->SaveValue();
              }

              break;
            }
            case 2:
            {
              FrameSpinBox *fsb = nullptr;
              if(!(fsb = dynamic_cast<FrameSpinBox*>(frame)))
                break;

              //Let's make sure that the value we're trying to save is different than what's on there already
              if(!IsClose(fsb->GetFrameValue(), default_value.second)){
                  fsb->SetFrameValue(default_value.second);
                  fsb->SaveValue();
              }
            }
          }
        }else{ //we didn't find the value
          iv.pcon->AddToLog("Failed to find target value on the module: " + QString(default_value.first.c_str()));
        }
      }
}

bool Tab::SaveSpecialDefaults(std::map<std::string,double> default_value_map){

    bool retVal = false;

    //Check to see which special defaults are in here, and call the correct functions to set them up
    bool (Tab::* SpecialFunctionPointers [SPECIAL_DEFAULTS.size()])(double value);

    SpecialFunctionPointers[0] = &Tab::SetNewBaudRate;

    //Go through each of the values in the map
    for(std::pair<std::string, double> default_value : default_value_map) {
        //Pull out the frame from this tab
        QString current_frame = default_value.first.c_str();
        for(int i = 0; i < SPECIAL_DEFAULTS.size(); i++){

            //Call the correct function for the given special variables
            if(current_frame == SPECIAL_DEFAULTS[i]){
                retVal |= ((*this).*(SpecialFunctionPointers[i]))(default_value.second);
            }
        }
    }

    return retVal;
}

bool Tab::SetNewBaudRate(double value){
    //We know the frame type
    bool baud_changed = false;
    Frame * baud_rate_frame = frame_map_["UART Baud Rate"];

    FrameSpinBox *fsb = nullptr;
    if(!(fsb = dynamic_cast<FrameSpinBox*>(baud_rate_frame))){
        return false;
    }

    //Let's make sure that the value we're trying to save is different than what's on there already
    if(!IsClose(fsb->GetFrameValue(), value)){
        fsb->SetFrameValue(value);
        fsb->SaveValue();
        baud_changed = true;
    }

    return baud_changed;
}

std::map<std::string,Frame*> Tab::get_frame_map(){
    return frame_map_;
}

std::map<std::string,FrameVariables*> Tab::get_frame_variables_map(){
    return frame_variables_map_;
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

void Tab::ConnectFrameButton(FrameButton *fb)
{
  connect(fb->push_button_set_, SIGNAL(clicked()), fb, SLOT(SetValue()));
}

void Tab::ConnectFrameReadOnly(FrameReadOnly *fr)
{
    connect(fr->push_button_get_, SIGNAL(clicked()), fr, SLOT(GetSavedReadOnlyValue()));
    connect(fr->spin_box_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), fr, &FrameReadOnly::SpinBoxValueChanged);
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
