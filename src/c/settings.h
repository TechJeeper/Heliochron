#pragma once

#include <pebble.h>
#include <stdbool.h>

typedef enum {
  TIME_FMT_SYSTEM = 0,
  TIME_FMT_12H,
  TIME_FMT_24H
} TimeFormat;

typedef enum {
  THEME_CLASSIC = 0,
  THEME_EMBER,
  THEME_ICE,
  THEME_NEON
} ThemeId;

typedef enum {
  HOUR_BODY_MARS = 0,
  HOUR_BODY_SATURN,
  HOUR_BODY_JUPITER
} HourBody;

typedef enum {
  ORBIT_CIRCULAR = 0,
  ORBIT_ELLIPTICAL
} OrbitStyle;

typedef struct {
  TimeFormat time_format;
  bool leading_zero;
  bool show_digital;
  bool show_date;
  bool show_seconds;
  bool show_orbits;
  bool show_starfield;
  bool show_battery_arc;
  bool twinkle;
  bool hourly_vibe;
  ThemeId theme;
  HourBody hour_body;
  OrbitStyle orbit_style;
} FaceSettings;

typedef void (*SettingsChangedFn)(void);

void settings_init(void);
void settings_deinit(void);
void settings_set_changed_callback(SettingsChangedFn cb);
const FaceSettings *settings_get(void);
