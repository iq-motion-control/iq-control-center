#ifndef INDICATION_HANDLER_HPP
#define INDICATION_HANDLER_HPP

#include "qserial_interface.h"
#include "IQ_api/client.hpp"
#include <math.h>

#define USING_JAZZY_PHRASE

class IndicationHandler : public QObject {
  Q_OBJECT

 public:
  IndicationHandler(QSerialInterface * ser, std::string path_to_clients);
  void PlayIndication();
  void UpdateBuzzerObjId(uint8_t obj_id);

 private:
  //notes that we can use (rounded...so they're not exactly the right note)
  const uint16_t c = (uint16_t)floor(261.6 * 2);
  const uint16_t low_c = (uint16_t)floor(261.6);
  const uint16_t d = (uint16_t)floor(293.665 * 2);
  const uint16_t e = (uint16_t)floor(329.7 * 2);
  const uint16_t low_e = (uint16_t)floor(329.7);
  const uint16_t f = (uint16_t)floor(349.2 * 2);
  const uint16_t low_f = (uint16_t)floor(349.2);
  const uint16_t g = (uint16_t)floor(392 * 2);
  const uint16_t low_g = (uint16_t)floor(392);
  const uint16_t a = (uint16_t)floor(440 * 2);
  const uint16_t b = (uint16_t)floor(493 * 2);
  const uint16_t low_b = (uint16_t)floor(493);
  const uint16_t ab = (uint16_t)floor(207.65);
  const uint16_t high_ab = (uint16_t)floor(207.65 * 2);
  const uint16_t db = (uint16_t)floor(277.18);
  const uint16_t eb = (uint16_t)floor(311.127);
  const uint16_t bb = (uint16_t)floor(466.164);
  const uint16_t low_bb = (uint16_t)(floor(bb / 2));
  const uint16_t fs = (uint16_t)floor(369.99);
  const uint16_t high_fs = (uint16_t)floor(369.99 * 2);

#ifdef USING_BB_TRIAD
  const uint16_t indication_speed = 100;
  const uint16_t indication_notes[9] = {bb, d, f, d, bb, d, f, d, bb};
  const uint16_t indication_durations[9] = {indication_speed, indication_speed, indication_speed, indication_speed, indication_speed, indication_speed, indication_speed, indication_speed, indication_speed};
#elif defined(USING_FOLKSONG)
  const uint16_t indication_speed = 125;
  const uint16_t indication_notes[5] = {d,e,d,e,g};
  const uint16_t indication_durations[5] = {indication_speed, indication_speed, indication_speed, indication_speed, indication_speed};
#elif defined(USING_FANCY_CHORD)
  const uint16_t indication_speed = 120;
  const uint16_t indication_notes[6] = {low_c, low_e, low_g, low_b, d, high_fs};
  const uint16_t indication_durations[6] = {indication_speed, indication_speed, indication_speed, indication_speed, indication_speed, indication_speed};
#elif defined(USING_BLUES_SCALE)
  const uint16_t indication_speed = 100;
  const uint16_t indication_notes[7] = {low_bb, db, eb, low_e, low_f, high_ab, bb};
  const uint16_t indication_durations[7] = {indication_speed,indication_speed,indication_speed,indication_speed,indication_speed,indication_speed,indication_speed};
#elif defined(USING_JAZZY_PHRASE)
  const uint16_t indication_speed_straight = 100;
  const uint16_t indication_speed_swung = 150;
  const uint16_t longer_note = 200;

  const uint16_t indication_notes[7] = {d, e, f, g, e, c, d};
  const uint16_t indication_durations[7] = {indication_speed_swung, indication_speed_straight, indication_speed_swung, indication_speed_straight, longer_note, indication_speed_swung, longer_note};
#endif


  void PlayNote(uint16_t frequency, uint16_t duration);

  QSerialInterface * serial_connection_;
  std::string path_to_clients_;
  std::map<std::string, Client *> buzzer_control_map_;
};

#endif // INDICATION_HANDLER_HPP
