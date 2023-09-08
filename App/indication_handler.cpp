
#include "indication_handler.hpp"
#include "IQ_api/client.hpp"
#include "IQ_api/client_helpers.hpp"

#include <QTime>
#include <QTimer>

IndicationHandler::IndicationHandler(QSerialInterface * ser, std::string path_to_clients) :
    serial_connection_(ser),
    path_to_clients_(path_to_clients)
{
  buzzer_control_map_ = ClientsFromJson(0, "buzzer_control_client.json", path_to_clients_, nullptr, nullptr);
}

void IndicationHandler::PlayIndication(){
  if(serial_connection_->ser_port_->isOpen()){
    for(uint8_t i = 0; i < sizeof(indication_notes)/sizeof(indication_notes[0]); i++){
      PlayNote(indication_notes[i], indication_durations[i]);
    }

    buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "ctrl_coast");
  }
}

void IndicationHandler::PlayNote(uint16_t frequency, uint16_t duration){
  int response = 25;

  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "hz", frequency);
  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "duration", duration);
  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "volume", 127);
  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "ctrl_note");

  //Keep checking for when the current note is done playing
  while(response > -1){
    GetEntryReply(*serial_connection_, buzzer_control_map_["buzzer_control_client"], "ctrl_mode", 5, 0.005f, response);
  }
}

