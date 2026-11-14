#include "face.h"
#include "settings.h"

static Window *s_window;
static Layer *s_layer;

static GBitmap *s_starfield;
static GBitmap *s_sun;
static GBitmap *s_mercury;
static GBitmap *s_earth;
static GBitmap *s_mars;
static GBitmap *s_saturn;
static GBitmap *s_jupiter;

static int s_hour;
static int s_minute;
static int s_second;
static int s_wday;
static int s_mday;
static int s_mon;
static int s_pulse;
static int s_battery_pct = 100;
static bool s_charging;
static bool s_bt_connected = true;
static int s_last_vibe_hour = -1;
static bool s_time_ready = false;

typedef struct {
  GColor bg;
  GColor orbit;
  GColor text;
  GColor accent;
  GColor battery;
  GColor dim;
} ThemeColors;

static ThemeColors theme_colors(ThemeId id) {
  ThemeColors t;
  switch (id) {
    case THEME_EMBER:
      t.bg = GColorBlack;
      t.orbit = GColorDarkCandyAppleRed;
      t.text = GColorChromeYellow;
      t.accent = GColorRed;
      t.battery = GColorOrange;
      t.dim = GColorDarkGray;
      break;
    case THEME_ICE:
      t.bg = GColorBlack;
      t.orbit = GColorPictonBlue;
      t.text = GColorWhite;
      t.accent = GColorCeleste;
      t.battery = GColorVividCerulean;
      t.dim = GColorDarkGray;
      break;
    case THEME_NEON:
      t.bg = GColorBlack;
      t.orbit = GColorFashionMagenta;
      t.text = GColorBrightGreen;
      t.accent = GColorCyan;
      t.battery = GColorMagenta;
      t.dim = GColorDarkGray;
      break;
    case THEME_CLASSIC:
    default:
      t.bg = GColorBlack;
      t.orbit = GColorOxfordBlue;
      t.text = GColorWhite;
      t.accent = GColorChromeYellow;
      t.battery = GColorIslamicGreen;
      t.dim = GColorDarkGray;
      break;
  }
  return t;
}

static bool use_24h(void) {
  const FaceSettings *s = settings_get();
  if (s->time_format == TIME_FMT_24H) return true;
  if (s->time_format == TIME_FMT_12H) return false;
  return clock_is_24h_style();
}

static void format_time(char *buf, size_t n) {
  int hour = s_hour;
  const bool is24 = use_24h();
  if (!is24) {
    hour = hour % 12;
    if (hour == 0) hour = 12;
  }
  if (settings_get()->leading_zero) {
    snprintf(buf, n, "%02d:%02d", hour, s_minute);
  } else {
    snprintf(buf, n, "%d:%02d", hour, s_minute);
  }
}

static void format_date(char *buf, size_t n) {
  static const char *const DOW[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
  static const char *const MON[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };
  snprintf(buf, n, "%s %d %s", DOW[s_wday % 7], s_mday, MON[s_mon % 12]);
}

static int32_t hour_angle(void) {
  // Smooth hour motion using minutes; 12h clock circle.
  int hour12 = s_hour % 12;
  return (hour12 * TRIG_MAX_ANGLE) / 12 + (s_minute * TRIG_MAX_ANGLE) / (12 * 60);
}

static int32_t minute_angle(void) {
  return (s_minute * TRIG_MAX_ANGLE) / 60 + (s_second * TRIG_MAX_ANGLE) / (60 * 60);
}

static int32_t second_angle(void) {
  return (s_second * TRIG_MAX_ANGLE) / 60;
}

static GPoint orbit_point(GPoint center, int rx, int ry, int32_t angle) {
  // 0 angle = 12 o'clock (top), clockwise like a clock.
  int32_t a = angle - TRIG_MAX_ANGLE / 4;
  return GPoint(
      center.x + (int16_t)(cos_lookup(a) * rx / TRIG_MAX_RATIO),
      center.y + (int16_t)(sin_lookup(a) * ry / TRIG_MAX_RATIO));
}

static void draw_orbit_ring(GContext *ctx, GPoint c, int rx, int ry, GColor color) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 1);
  if (rx == ry) {
    graphics_draw_circle(ctx, c, rx);
    return;
  }
  // Approximate ellipse with short chord segments.
  GPoint prev = orbit_point(c, rx, ry, 0);
  for (int i = 1; i <= 48; i++) {
    int32_t ang = (i * TRIG_MAX_ANGLE) / 48;
    GPoint p = orbit_point(c, rx, ry, ang);
    graphics_draw_line(ctx, prev, p);
    prev = p;
  }
}

static void draw_bitmap_centered(GContext *ctx, GBitmap *bmp, GPoint center) {
  if (!bmp) return;
  GSize sz = gbitmap_get_bounds(bmp).size;
  GRect dest = GRect(center.x - sz.w / 2, center.y - sz.h / 2, sz.w, sz.h);
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, bmp, dest);
}

static void draw_twinkles(GContext *ctx, GRect bounds, GColor color) {
  // Deterministic sparkle field that shifts with pulse.
  static const int8_t xs[] = {18, 42, 70, 95, 120, 148, 172, 28, 160, 88, 55, 185};
  static const int8_t ys[] = {22, 48, 16, 70, 34, 58, 90, 110, 130, 150, 175, 200};
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_fill_color(ctx, color);
  for (int i = 0; i < (int)(sizeof(xs) / sizeof(xs[0])); i++) {
    if (((s_pulse + i * 3) % 7) > 4) continue;
    int x = xs[i] % bounds.size.w;
    int y = ys[i] % bounds.size.h;
    graphics_draw_pixel(ctx, GPoint(x, y));
    if ((i + s_pulse) % 5 == 0) {
      graphics_draw_pixel(ctx, GPoint(x + 1, y));
      graphics_draw_pixel(ctx, GPoint(x, y + 1));
    }
  }
}

static void draw_battery_arc(GContext *ctx, GPoint c, int r, GColor color) {
  int pct = s_battery_pct;
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  int32_t span = (pct * TRIG_MAX_ANGLE) / 100;
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_stroke_width(ctx, 2);
  GPoint prev = orbit_point(c, r, r, 0);
  const int steps = 36;
  for (int i = 1; i <= steps; i++) {
    int32_t ang = (i * span) / steps;
    GPoint p = orbit_point(c, r, r, ang);
    graphics_draw_line(ctx, prev, p);
    prev = p;
  }
  if (s_charging) {
    graphics_context_set_fill_color(ctx, color);
    GPoint tip = orbit_point(c, r, r, span);
    graphics_fill_circle(ctx, tip, 2);
  }
}

static GBitmap *hour_bitmap(void) {
  switch (settings_get()->hour_body) {
    case HOUR_BODY_SATURN: return s_saturn;
    case HOUR_BODY_JUPITER: return s_jupiter;
    case HOUR_BODY_MARS:
    default: return s_mars;
  }
}

static void update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  const FaceSettings *cfg = settings_get();
  ThemeColors theme = theme_colors(cfg->theme);

  // Caption band first — solar system is laid out entirely above it.
#if defined(PBL_ROUND)
  const int caption_h = (cfg->show_digital ? 36 : 0) + (cfg->show_date ? 22 : 0) + 8;
  const int caption_pad = 18;
  GFont time_font = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
  GFont date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  const int time_step = 34;
#elif defined(PBL_PLATFORM_EMERY)
  const int caption_h = (cfg->show_digital ? 34 : 0) + (cfg->show_date ? 20 : 0) + 6;
  const int caption_pad = 4;
  GFont time_font = fonts_get_system_font(FONT_KEY_LECO_32_BOLD_NUMBERS);
  GFont date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  const int time_step = 32;
#else
  const int caption_h = (cfg->show_digital ? 28 : 0) + (cfg->show_date ? 18 : 0) + 4;
  const int caption_pad = 2;
  GFont time_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
  GFont date_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  const int time_step = 26;
#endif

  const bool show_captions = cfg->show_digital || cfg->show_date;
  const int caption_top = show_captions
      ? (bounds.size.h - caption_h - caption_pad)
      : bounds.size.h;
  const int time_y = caption_top;
  const int date_y = cfg->show_digital ? caption_top + time_step : caption_top + 4;

  // Orrery lives in the band above the captions so planets are never covered.
  const int system_top = PBL_IF_ROUND_ELSE(10, 2);
  const int system_bottom = show_captions ? (caption_top - 6) : bounds.size.h - 4;
  const int system_h = system_bottom - system_top;
  GPoint center = GPoint(bounds.size.w / 2, system_top + system_h / 2);

  graphics_context_set_fill_color(ctx, theme.bg);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  if (cfg->show_starfield && s_starfield) {
    graphics_context_set_compositing_mode(ctx, GCompOpAssign);
    GSize sf = gbitmap_get_bounds(s_starfield).size;
    // Draw at native size centered — Pebble does not upscale bitmaps.
    GRect dest = GRect(
        (bounds.size.w - sf.w) / 2,
        (bounds.size.h - sf.h) / 2,
        sf.w,
        sf.h);
    graphics_draw_bitmap_in_rect(ctx, s_starfield, dest);
  }

  if (cfg->twinkle) {
    draw_twinkles(ctx, bounds, theme.accent);
  }

  const int half_w = bounds.size.w / 2 - PBL_IF_ROUND_ELSE(12, 6);
  const int half_h = system_h / 2 - 4;
  const int max_r = (half_w < half_h ? half_w : half_h);
  int sec_rx = max_r * 38 / 100;
  int min_rx = max_r * 58 / 100;
  int hour_rx = max_r * 82 / 100;
  int sec_ry = sec_rx;
  int min_ry = min_rx;
  int hour_ry = hour_rx;
  if (cfg->orbit_style == ORBIT_ELLIPTICAL) {
    sec_ry = sec_rx * 78 / 100;
    min_ry = min_rx * 78 / 100;
    hour_ry = hour_rx * 78 / 100;
  }

  if (cfg->show_orbits) {
    draw_orbit_ring(ctx, center, sec_rx, sec_ry, theme.orbit);
    draw_orbit_ring(ctx, center, min_rx, min_ry, theme.orbit);
    draw_orbit_ring(ctx, center, hour_rx, hour_ry, theme.orbit);
  }

  // Optional battery arc — kept inside the system band (not the full dial).
  if (cfg->show_battery_arc) {
    draw_battery_arc(ctx, center, max_r - 2, theme.battery);
  }

  draw_bitmap_centered(ctx, s_sun, center);

  GPoint hour_pt = orbit_point(center, hour_rx, hour_ry, hour_angle());
  GPoint min_pt = orbit_point(center, min_rx, min_ry, minute_angle());
  draw_bitmap_centered(ctx, hour_bitmap(), hour_pt);
  draw_bitmap_centered(ctx, s_earth, min_pt);

  if (cfg->show_seconds) {
    GPoint sec_pt = orbit_point(center, sec_rx, sec_ry, second_angle());
    draw_bitmap_centered(ctx, s_mercury, sec_pt);
  }

  if (show_captions) {
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx,
                       GRect(bounds.size.w / 8, caption_top - 2,
                             bounds.size.w * 3 / 4, caption_h + 4),
                       4, GCornersAll);
  }

  if (cfg->show_digital) {
    char time_buf[16];
    format_time(time_buf, sizeof(time_buf));
    graphics_context_set_text_color(ctx, theme.text);
    graphics_draw_text(ctx, time_buf, time_font,
                       GRect(0, time_y, bounds.size.w, 36),
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, NULL);
  }

  if (cfg->show_date) {
    char date_buf[24];
    format_date(date_buf, sizeof(date_buf));
    // Use accent (bright) — theme.dim vanishes on the starfield.
    graphics_context_set_text_color(ctx, theme.accent);
    graphics_draw_text(ctx, date_buf, date_font,
                       GRect(0, date_y, bounds.size.w, 22),
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentCenter, NULL);
  }

  if (!s_bt_connected) {
    graphics_context_set_fill_color(ctx, theme.accent);
#if defined(PBL_ROUND)
    // Near top-center — corners are clipped on round displays.
    graphics_fill_circle(ctx, GPoint(bounds.size.w / 2, 16), 3);
#else
    graphics_fill_circle(ctx, GPoint(bounds.size.w - 8, 8), 3);
#endif
  }
}

static void unload_bitmaps(void) {
  if (s_starfield) { gbitmap_destroy(s_starfield); s_starfield = NULL; }
  if (s_sun) { gbitmap_destroy(s_sun); s_sun = NULL; }
  if (s_mercury) { gbitmap_destroy(s_mercury); s_mercury = NULL; }
  if (s_earth) { gbitmap_destroy(s_earth); s_earth = NULL; }
  if (s_mars) { gbitmap_destroy(s_mars); s_mars = NULL; }
  if (s_saturn) { gbitmap_destroy(s_saturn); s_saturn = NULL; }
  if (s_jupiter) { gbitmap_destroy(s_jupiter); s_jupiter = NULL; }
}

static void load_bitmaps(void) {
  if (s_sun) return;
  s_starfield = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STARFIELD);
  s_sun = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SUN);
  s_mercury = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MERCURY);
  s_earth = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EARTH);
  s_mars = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MARS);
  s_saturn = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SATURN);
  s_jupiter = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_JUPITER);
}

static void battery_handler(BatteryChargeState state) {
  s_battery_pct = state.charge_percent;
  s_charging = state.is_charging;
  if (s_layer) layer_mark_dirty(s_layer);
}

static void bt_handler(bool connected) {
  s_bt_connected = connected;
  if (s_layer) layer_mark_dirty(s_layer);
}

void face_init(Window *window) {
  s_window = window;
  load_bitmaps();

  Layer *root = window_get_root_layer(window);
  s_layer = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_layer, update_proc);
  layer_add_child(root, s_layer);

  BatteryChargeState bat = battery_state_service_peek();
  s_battery_pct = bat.charge_percent;
  s_charging = bat.is_charging;
  battery_state_service_subscribe(battery_handler);

  s_bt_connected = connection_service_peek_pebble_app_connection();
  connection_service_subscribe((ConnectionHandlers){
    .pebble_app_connection_handler = bt_handler
  });

  face_tick();
}

void face_deinit(void) {
  battery_state_service_unsubscribe();
  connection_service_unsubscribe();
  if (s_layer) {
    layer_destroy(s_layer);
    s_layer = NULL;
  }
  unload_bitmaps();
  s_window = NULL;
}

void face_tick(void) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int prev_hour = s_hour;
  s_hour = t->tm_hour;
  s_minute = t->tm_min;
  s_second = t->tm_sec;
  s_wday = t->tm_wday;
  s_mday = t->tm_mday;
  s_mon = t->tm_mon;

  if (s_time_ready && settings_get()->hourly_vibe &&
      s_hour != prev_hour && s_hour != s_last_vibe_hour) {
    vibes_short_pulse();
    s_last_vibe_hour = s_hour;
  }
  s_time_ready = true;

  if (s_layer) layer_mark_dirty(s_layer);
}

void face_pulse(void) {
  s_pulse = (s_pulse + 1) % 64;
  if (settings_get()->twinkle && s_layer) {
    layer_mark_dirty(s_layer);
  }
}

void face_apply_settings(void) {
  if (s_window) {
    window_set_background_color(s_window, theme_colors(settings_get()->theme).bg);
  }
  if (s_layer) layer_mark_dirty(s_layer);
}
