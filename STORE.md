# Rebble Appstore listing — Heliochron

Copy these fields into [dev-portal.rebble.io](https://dev-portal.rebble.io/submit).

| Field | Value |
| --- | --- |
| App name | Heliochron |
| Type | Watchface |
| Category | Faces *(automatic)* |
| Website | *(optional)* |
| Source code | https://github.com/TechJeeper/Heliochron |
| `.pbw` | `build/Heliochron.pbw` |
| CloudPebble zip | `dist/Heliochron-cloudpebble.zip` |
| Platforms | Pebble Time 2 (emery), Time / Time Steel (basalt), Round 2 (gabbro) |

## Description

Paste into each platform asset collection (max 1600 characters):

```
A living orrery on your wrist. Heliochron turns time into a solar system: the Sun holds the center while planets keep the clock.

• Mars, Saturn, or Jupiter tracks the hours
• Earth sweeps the minutes
• Mercury races the seconds (optional, to save battery)
• Circular or elliptical orbits
• PixelLab starfield + planet sprites
• Four looks: Classic Cosmic, Ember Void, Ice Nebula, Neon Drift
• Optional digital time, date, orbit rings, twinkling stars
• Battery comet arc around the outer rim
• Hourly vibrate
• Clay phone settings

Designed for Pebble Time 2 and Pebble Round 2; also runs on Pebble Time / Time Steel.
```

## Short blurb (optional)

```
Solar system watchface — planets keep the hours, minutes, and seconds around a PixelLab sun.
```

## Release notes (v1.1.1)

```
Larger digital time (LECO) and high-contrast date on a dark scrim so captions stay readable on the starfield.
```

## Release notes (v1.1.0)

```
Add Pebble Round 2 (gabbro) support: 260×260 starfield, round-safe layout, and store screenshots.
```

## Screenshots

Exact platform sizes (no watch bezel). Regenerate with:

```powershell
python tools/generate_store_screenshots.py
```

| Platform | Size | Folder |
| --- | --- | --- |
| basalt (Pebble Time) | 144×168 | `store/screenshots/basalt/` |
| emery (Pebble Time 2) | 200×228 | `store/screenshots/emery/` |
| gabbro (Pebble Round 2) | 260×260 | `store/screenshots/gabbro/` |

Five shots per platform:

1. Classic Cosmic · Mars hours · date  
2. Ember Void · Saturn hours  
3. Ice Nebula · Jupiter hours · date  
4. Neon Drift · elliptical orbits  
5. Minimal · planets only (no digital / rings)

## Listing checklist

- [ ] Screenshots uploaded per platform (up to 5)
- [ ] Description above on each asset collection
- [ ] Source URL set to the GitHub repo
- [ ] Publish `build/Heliochron.pbw` as release 1.1.0
