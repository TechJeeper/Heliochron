#include "settings.h"

enum {
  PERSIST_TIME_FORMAT = 1,
  PERSIST_LEADING_ZERO = 2,
  PERSIST_SHOW_DIGITAL = 3,
  PERSIST_SHOW_DATE = 4,
  PERSIST_SHOW_SECONDS = 5,
  PERSIST_SHOW_ORBITS = 6,
  PERSIST_SHOW_STARFIELD = 7,
  // Bumped key so older installs that defaulted the arc on pick up the new off default.
  PERSIST_SHOW_BATTERY = 18,
  PERSIST_TWINKLE = 9,
  PERSIST_THEME = 10,
  PERSIST_HOUR_BODY = 11,
  PERSIST_ORBIT_STYLE = 12,
  PERSIST_HOURLY_VIBE = 13,
};

static FaceSettings s_settings;
static SettingsChangedFn s_changed;
static bool s_inbox_open;

static int clamp_int(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

static void normalize(void) {
  s_settings.time_format = (TimeFormat)clamp_int(
      (int)s_settings.time_format, TIME_FMT_SYSTEM, TIME_FMT_24H);
  s_settings.theme = (ThemeId)clamp_int((int)s_settings.theme, THEME_CLASSIC, THEME_NEON);
  s_settings.hour_body = (HourBody)clamp_int(
      (int)s_settings.hour_body, HOUR_BODY_MARS, HOUR_BODY_JUPITER);
  s_settings.orbit_style = (OrbitStyle)clamp_int(
      (int)s_settings.orbit_style, ORBIT_CIRCULAR, ORBIT_ELLIPTICAL);
}

static void settings_set_defaults(void) {
  s_settings.time_format = TIME_FMT_SYSTEM;
  s_settings.leading_zero = true;
  s_settings.show_digital = true;
  s_settings.show_date = true;
  s_settings.show_seconds = true;
  s_settings.show_orbits = true;
  s_settings.show_starfield = true;
  s_settings.show_battery_arc = false;
  s_settings.twinkle = true;
  s_settings.hourly_vibe = false;
  s_settings.theme = THEME_CLASSIC;
  s_settings.hour_body = HOUR_BODY_MARS;
  s_settings.orbit_style = ORBIT_CIRCULAR;
}

static void settings_load(void) {
  settings_set_defaults();
  if (persist_exists(PERSIST_TIME_FORMAT)) {
    s_settings.time_format = (TimeFormat)persist_read_int(PERSIST_TIME_FORMAT);
  }
  if (persist_exists(PERSIST_LEADING_ZERO)) {
    s_settings.leading_zero = persist_read_bool(PERSIST_LEADING_ZERO);
  }
  if (persist_exists(PERSIST_SHOW_DIGITAL)) {
    s_settings.show_digital = persist_read_bool(PERSIST_SHOW_DIGITAL);
  }
  if (persist_exists(PERSIST_SHOW_DATE)) {
    s_settings.show_date = persist_read_bool(PERSIST_SHOW_DATE);
  }
  if (persist_exists(PERSIST_SHOW_SECONDS)) {
    s_settings.show_seconds = persist_read_bool(PERSIST_SHOW_SECONDS);
  }
  if (persist_exists(PERSIST_SHOW_ORBITS)) {
    s_settings.show_orbits = persist_read_bool(PERSIST_SHOW_ORBITS);
  }
  if (persist_exists(PERSIST_SHOW_STARFIELD)) {
    s_settings.show_starfield = persist_read_bool(PERSIST_SHOW_STARFIELD);
  }
  if (persist_exists(PERSIST_SHOW_BATTERY)) {
    s_settings.show_battery_arc = persist_read_bool(PERSIST_SHOW_BATTERY);
  }
  if (persist_exists(PERSIST_TWINKLE)) {
    s_settings.twinkle = persist_read_bool(PERSIST_TWINKLE);
  }
  if (persist_exists(PERSIST_THEME)) {
    s_settings.theme = (ThemeId)persist_read_int(PERSIST_THEME);
  }
  if (persist_exists(PERSIST_HOUR_BODY)) {
    s_settings.hour_body = (HourBody)persist_read_int(PERSIST_HOUR_BODY);
  }
  if (persist_exists(PERSIST_ORBIT_STYLE)) {
    s_settings.orbit_style = (OrbitStyle)persist_read_int(PERSIST_ORBIT_STYLE);
  }
  if (persist_exists(PERSIST_HOURLY_VIBE)) {
    s_settings.hourly_vibe = persist_read_bool(PERSIST_HOURLY_VIBE);
  }
  normalize();
}

static void settings_save(void) {
  normalize();
  persist_write_int(PERSIST_TIME_FORMAT, (int)s_settings.time_format);
  persist_write_bool(PERSIST_LEADING_ZERO, s_settings.leading_zero);
  persist_write_bool(PERSIST_SHOW_DIGITAL, s_settings.show_digital);
  persist_write_bool(PERSIST_SHOW_DATE, s_settings.show_date);
  persist_write_bool(PERSIST_SHOW_SECONDS, s_settings.show_seconds);
  persist_write_bool(PERSIST_SHOW_ORBITS, s_settings.show_orbits);
  persist_write_bool(PERSIST_SHOW_STARFIELD, s_settings.show_starfield);
  persist_write_bool(PERSIST_SHOW_BATTERY, s_settings.show_battery_arc);
  persist_write_bool(PERSIST_TWINKLE, s_settings.twinkle);
  persist_write_int(PERSIST_THEME, (int)s_settings.theme);
  persist_write_int(PERSIST_HOUR_BODY, (int)s_settings.hour_body);
  persist_write_int(PERSIST_ORBIT_STYLE, (int)s_settings.orbit_style);
  persist_write_bool(PERSIST_HOURLY_VIBE, s_settings.hourly_vibe);
}

static int tuple_as_int(Tuple *t, int fallback) {
  if (!t) return fallback;
  if (t->type == TUPLE_INT) return (int)t->value->int32;
  if (t->type == TUPLE_CSTRING) return atoi(t->value->cstring);
  return fallback;
}

static bool tuple_as_bool(Tuple *t, bool fallback) {
  if (!t) return fallback;
  if (t->type == TUPLE_INT) return t->value->int32 != 0;
  if (t->type == TUPLE_CSTRING) {
    return strcmp(t->value->cstring, "true") == 0 ||
           strcmp(t->value->cstring, "1") == 0;
  }
  return fallback;
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  (void)context;
  Tuple *t;

  t = dict_find(iter, MESSAGE_KEY_TimeFormat);
  if (t) s_settings.time_format = (TimeFormat)tuple_as_int(t, (int)s_settings.time_format);
  t = dict_find(iter, MESSAGE_KEY_LeadingZero);
  if (t) s_settings.leading_zero = tuple_as_bool(t, s_settings.leading_zero);
  t = dict_find(iter, MESSAGE_KEY_ShowDigital);
  if (t) s_settings.show_digital = tuple_as_bool(t, s_settings.show_digital);
  t = dict_find(iter, MESSAGE_KEY_ShowDate);
  if (t) s_settings.show_date = tuple_as_bool(t, s_settings.show_date);
  t = dict_find(iter, MESSAGE_KEY_ShowSeconds);
  if (t) s_settings.show_seconds = tuple_as_bool(t, s_settings.show_seconds);
  t = dict_find(iter, MESSAGE_KEY_ShowOrbits);
  if (t) s_settings.show_orbits = tuple_as_bool(t, s_settings.show_orbits);
  t = dict_find(iter, MESSAGE_KEY_ShowStarfield);
  if (t) s_settings.show_starfield = tuple_as_bool(t, s_settings.show_starfield);
  t = dict_find(iter, MESSAGE_KEY_ShowBatteryArc);
  if (t) s_settings.show_battery_arc = tuple_as_bool(t, s_settings.show_battery_arc);
  t = dict_find(iter, MESSAGE_KEY_Twinkle);
  if (t) s_settings.twinkle = tuple_as_bool(t, s_settings.twinkle);
  t = dict_find(iter, MESSAGE_KEY_Theme);
  if (t) s_settings.theme = (ThemeId)tuple_as_int(t, (int)s_settings.theme);
  t = dict_find(iter, MESSAGE_KEY_HourBody);
  if (t) s_settings.hour_body = (HourBody)tuple_as_int(t, (int)s_settings.hour_body);
  t = dict_find(iter, MESSAGE_KEY_OrbitStyle);
  if (t) s_settings.orbit_style = (OrbitStyle)tuple_as_int(t, (int)s_settings.orbit_style);
  t = dict_find(iter, MESSAGE_KEY_HourlyVibe);
  if (t) s_settings.hourly_vibe = tuple_as_bool(t, s_settings.hourly_vibe);

  settings_save();
  if (s_changed) {
    s_changed();
  }
}

void settings_init(void) {
  settings_load();
  if (!s_inbox_open) {
    app_message_register_inbox_received(inbox_received);
    app_message_open(app_message_inbox_size_maximum(),
                     app_message_outbox_size_maximum());
    s_inbox_open = true;
  }
}

void settings_deinit(void) {
  settings_save();
  if (s_inbox_open) {
    app_message_deregister_callbacks();
    s_inbox_open = false;
  }
}

void settings_set_changed_callback(SettingsChangedFn cb) {
  s_changed = cb;
}

const FaceSettings *settings_get(void) {
  return &s_settings;
}
