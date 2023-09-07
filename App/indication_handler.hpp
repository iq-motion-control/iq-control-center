#ifndef INDICATION_HANDLER_HPP
#define INDICATION_HANDLER_HPP

#include "qserial_interface.h"
#include "IQ_api/client.hpp"
#include "IQ_api/client_helpers.hpp"

class IndicationHandler : public QObject {
  Q_OBJECT

 public:
  IndicationHandler(QSerialInterface * ser, std::string path_to_clients);
  void PlayIndication();

 private:

  const uint16_t c = (uint16_t)floor(261.6 * 2);
  const uint16_t d = (uint16_t)floor(293.665 * 2);
  const uint16_t e = (uint16_t)floor(329.7 * 2);
  const uint16_t f = (uint16_t)floor(349.2 * 2);
  const uint16_t g = (uint16_t)floor(392 * 2);
  const uint16_t a = (uint16_t)floor(440 * 2);
  const uint16_t b = (uint16_t)floor(493 * 2);
  const uint16_t ab = (uint16_t)floor(207.65);
  const uint16_t db = (uint16_t)floor(277.18);
  const uint16_t eb = (uint16_t)floor(311.127);
  const uint16_t bb = (uint16_t)floor(466.164);
  const uint16_t fs = (uint16_t)floor(369.99);

  const uint16_t indication_eighth = 200;
  const uint16_t indication_swung_eighth = indication_eighth + (indication_eighth/2);
  const uint16_t indication_notes[8] = {f, f, (uint16_t)(ab * 4), (uint16_t)(ab * 4), f, c, (uint16_t)(eb*2), f};
  const uint16_t indication_durations[8] = {indication_swung_eighth, indication_eighth, indication_eighth, indication_eighth,indication_swung_eighth, indication_eighth, (uint16_t)(indication_eighth*2), (uint16_t)(indication_eighth*2)};

  void PlayNote(uint16_t frequency, uint16_t duration);

  QSerialInterface * serial_connection_;
  std::string path_to_clients_;
  std::map<std::string, Client *> buzzer_control_map_;
};

#endif // INDICATION_HANDLER_HPP
