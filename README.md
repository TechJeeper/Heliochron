# Heliochron

A solar-system watchface for **Pebble Time 2** (and Time / Time Steel) where the planets *are* the clock.

- **Sun** — center of the system  
- **Hour planet** — Mars, Saturn, or Jupiter (Clay setting)  
- **Earth** — minutes  
- **Mercury** — seconds (optional, for battery)  

Pixel-art bodies and starfield were generated with [PixelLab](https://pixellab.ai). Settings use [@rebble/clay](https://github.com/pebble-dev/clay).

## Build

```powershell
npm install
./build.ps1
```

Output: `build/Heliochron.pbw`

```powershell
./build.ps1 -Phone 192.168.1.23
./build.ps1 -Zip
```

Open the watchface settings from the Pebble/Rebble phone app to customize.

## Clay settings

| Setting | What it does |
|--------|----------------|
| Hour format | System / 12h / 24h |
| Leading zero | Pad digital hours |
| Show digital time | Numeric readout under the system |
| Show date | Weekday + date |
| Hourly vibrate | Short pulse on the hour |
| Hour planet | Mars / Saturn / Jupiter |
| Orbit shape | Circular or elliptical |
| Mercury second hand | Orbit Mercury for seconds |
| Orbit rings | Draw faint orbital paths |
| Starfield background | PixelLab nebula backdrop |
| Battery comet arc | Charge level as an outer arc |
| Twinkling stars | Soft sparkle overlay |
| Theme | Classic Cosmic, Ember Void, Ice Nebula, Neon Drift |

## CloudPebble / store

```powershell
./build.ps1 -Zip
python tools/generate_store_screenshots.py
```

- Import zip: `dist/Heliochron-cloudpebble.zip`
- Store copy + screenshot notes: [`STORE.md`](STORE.md)
- Screenshots: `store/screenshots/{basalt,emery}/`

## Name note

**Heliochron** (historically a solar timepiece / sundial) was checked against the Rebble/Pebble app store and is not used by an existing watchface listing.
