#ifndef CONFIG_H
#define CONFIG_H

#include <EEPROM.h>

#define CONFIG_MAGIC 0xABCD1234
#define EEPROM_SIZE 256

struct Config {
  uint32_t magic;
  char ssid[33];
  char password[65];
  char rtdbHost[129];
  char rtdbSecret[65];
};

static Config cfg;

inline void configDefaults() {
  cfg.magic = CONFIG_MAGIC;
  memset(cfg.ssid, 0, sizeof(cfg.ssid));
  memset(cfg.password, 0, sizeof(cfg.password));
  memset(cfg.rtdbHost, 0, sizeof(cfg.rtdbHost));
  memset(cfg.rtdbSecret, 0, sizeof(cfg.rtdbSecret));
  strncpy(cfg.ssid, "Repeater-9924", sizeof(cfg.ssid) - 1);
  strncpy(cfg.password, "nino2222", sizeof(cfg.password) - 1);
}

inline bool configValid() {
  return cfg.magic == CONFIG_MAGIC && strlen(cfg.ssid) > 0;
}

inline void configLoad() {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, cfg);
  if (!configValid()) {
    configDefaults();
    configSave(); // Save defaults to EEPROM on first boot
  }
}

inline void configSave() {
  cfg.magic = CONFIG_MAGIC;
  EEPROM.put(0, cfg);
  EEPROM.commit();
}

inline void configClear() {
  configDefaults();
  EEPROM.put(0, cfg);
  EEPROM.commit();
}

#endif
