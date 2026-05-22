# K-Balabolka

macOS port of [Balabolka](https://www.cross-plus-a.com/balabolka.htm), the popular Windows text-to-speech reader.

## Architecture

```
┌──────────────────────────┐
│   PySide6 GUI (Python)   │
└─────────────┬────────────┘
              │
┌─────────────▼────────────┐
│   GStreamer pipeline     │
└─────────────┬────────────┘
              │
┌─────────────▼────────────┐
│  macttssink (C plugin)   │ ← AVSpeechSynthesizer (macOS native TTS)
└──────────────────────────┘
```

- **GUI**: PySide6
- **Audio/TTS pipeline**: GStreamer 1.28+
- **TTS plugin (`macttssink`)**: Custom GStreamer sink written in C, modelled on [gst-ttssink](https://github.com/avstack/gst-ttssink) (Rust reference)
- **TTS backend**: `AVSpeechSynthesizer` from AVFoundation

## Status

Built incrementally, one stage at a time:

- [x] **Stage 1** — Skeleton plugin: empty `GstBaseSink` subclass, `render()` prints received text to stdout
- [x] **Stage 2** — `AVSpeechSynthesizer` wired into `render()` via an Objective-C bridge; element renamed `ttssink` → `macttssink`
- [ ] **Stage 3** — Properties: rate, voice, pitch
- [ ] **Stage 4** — PySide6 GUI integration

## Build

Requires the official [GStreamer .pkg](https://gstreamer.freedesktop.org/download/) installed at `/Library/Frameworks/GStreamer.framework/`, plus Xcode Command Line Tools, Meson, and Ninja.

```sh
cd plugin
PKG_CONFIG_PATH= \
  PKG_CONFIG_LIBDIR=/Library/Frameworks/GStreamer.framework/Versions/1.0/lib/pkgconfig \
  meson setup builddir

PKG_CONFIG_PATH= \
  PKG_CONFIG_LIBDIR=/Library/Frameworks/GStreamer.framework/Versions/1.0/lib/pkgconfig \
  ninja -C builddir
```

> **Note:** clearing `PKG_CONFIG_PATH` prevents Homebrew's GLib from being linked alongside the Framework's GLib, which would cause `GObject: NODE_REFCOUNT` crashes at plugin load time.

## Test

```sh
echo "안녕하세요" | \
  GST_PLUGIN_PATH=$(pwd)/plugin/builddir \
  gst-launch-1.0 --quiet fdsrc ! 'text/x-raw,format=utf8' ! macttssink
```

Speaks the text through your Mac's speakers using the current system voice.

## License

TBD.
