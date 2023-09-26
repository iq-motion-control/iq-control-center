#include "indication_handler.hpp"
#include "IQ_api/client.hpp"
#include "IQ_api/client_helpers.hpp"
#include "main.h"
#include <QDateTime>
#include <QThread>

IndicationHandler::IndicationHandler(QSerialInterface * ser, std::string path_to_clients):
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
  int current_state;

  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "hz", frequency);
  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "duration", duration);
  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "volume", 70);
  buzzer_control_map_["buzzer_control_client"]->Set(*serial_connection_, "ctrl_note");

  serial_connection_->SendNow(); // Use SendNow() as opposed to GetEntryReply() to immediately send the Set commands

  // First check if the module's buzzer state is "in note" meaning the song has begun playing.
  qint64 timeout = 1000; // 1 second timeout while waiting for response from module to prevent Control Center from waiting indefinitely for the module's response
  current_state = WaitForBuzzerState(MODULE_BUZZER_IN_NOTE, timeout);

  // Then check if the module's buzzer state is in "no change" meaning the song has finished playing.
  if(current_state == MODULE_BUZZER_IN_NOTE){
    WaitForBuzzerState(MODULE_BUZZER_IN_NO_CHANGE, timeout);
  }
  QThread::msleep(1); // Wait 1 millisecond to allow module to cleanup before moving on to next note (avoid slippage).
}

int IndicationHandler::WaitForBuzzerState(int target_state, qint64 timeout){
  qint64 start_time = QDateTime::currentMSecsSinceEpoch();
  qint64 end_time = start_time + timeout;
  qint64 current_time;

  int current_state;

  do{
    current_time = QDateTime::currentMSecsSinceEpoch();
    GetEntryReply(*serial_connection_, buzzer_control_map_["buzzer_control_client"], "ctrl_mode", 1, 0.01f, current_state);
  }
  while (current_state != target_state && current_time < end_time);

  return current_state;
}

void IndicationHandler::UpdateBuzzerObjId(uint8_t obj_id){
  buzzer_control_map_["buzzer_control_client"]->UpdateClientObjId(obj_id);
}
