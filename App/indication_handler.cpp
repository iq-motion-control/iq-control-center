#include "indication_handler.hpp"
#include "IQ_api/client.hpp"
#include "IQ_api/client_helpers.hpp"
#include "main.h"
#include <QDateTime>

IndicationHandler::IndicationHandler(QSerialInterface * ser, std::string path_to_clients) :
    serial_connection_(ser),
    path_to_clients_(path_to_clients)
{
  buzzer_control_map_ = ClientsFromJson(0, "buzzer_control_client.json", path_to_clients_, nullptr, nullptr);
}

void IndicationHandler::PlayIndication(){
  if(iv.pcon->GetConnectionState()){
    for(uint8_t i = 0; i < sizeof(indication_notes)/sizeof(indication_notes[0]); i++){
      PlayNote(indication_notes[i], indication_durations[i]);
    }

    buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "ctrl_coast");
  }
}

void IndicationHandler::PlayNote(uint16_t frequency, uint16_t duration){
  int response = MODULE_BUZZER_IN_NO_CHANGE;

  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "hz", frequency);
  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "duration", duration);
  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "volume", 70);
  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "ctrl_note");

  // Set timeout for 1 second to prevent Control Center from waiting indefinitely for the module's response
  qint64 start_time = QDateTime::currentSecsSinceEpoch();
  qint64 end_time = start_time + 1; // 1 second timeout

  // First check if the module's buzzer state is "in note" meaning the song has begun playing
  while (response < MODULE_BUZZER_IN_NOTE) {
    qint64 current_time = QDateTime::currentSecsSinceEpoch();
    GetEntryReply(*serial_connection_, buzzer_control_map_["buzzer_control_client"], "ctrl_mode", 1, 0.01f, response);
    if(current_time > end_time) {
      break; // break out of while loop if timeout condition is met
    }
  }

  // Reset start and end time for next while loop
  start_time = QDateTime::currentSecsSinceEpoch();
  end_time = start_time + 1;
  // Keep checking for when the current note is done playing
  while(response > MODULE_BUZZER_IN_NO_CHANGE){
    qint64 current_time = QDateTime::currentSecsSinceEpoch();
    GetEntryReply(*serial_connection_, buzzer_control_map_["buzzer_control_client"], "ctrl_mode", 1, 0.01f, response);
    if(current_time > end_time) {
      break;
    }
  }
}

void IndicationHandler::UpdateBuzzerObjId(uint8_t obj_id){
  buzzer_control_map_["buzzer_control_client"]->UpdateClientObjId(obj_id);
}

