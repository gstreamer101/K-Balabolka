// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (c) 2026 dlgus8648

#ifndef GSTMACTTSSINK_MACOS_H
#define GSTMACTTSSINK_MACOS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle. Created by mac_tts_init(), destroyed by mac_tts_shutdown(). */
typedef struct mac_tts_ctx mac_tts_ctx;

/* Utterance options. Match AVSpeechUtterance properties.
 *   rate:     0.0..1.0  (AVSpeech default ≈ 0.5)
 *   pitch:    0.5..2.0  (default 1.0)
 *   volume:   0.0..1.0  (default 1.0)
 *   voice_id: NULL = system default voice for current locale
 *             "ko-KR" / "en-US" style language code, OR
 *             "com.apple.voice.compact.ko-KR.Yuna" style identifier
 */
typedef struct {
  float       rate;
  float       pitch;
  float       volume;
  const char *voice_id;
} mac_tts_options;

#define MAC_TTS_OPTIONS_DEFAULTS { 0.5f, 1.0f, 1.0f, NULL }

mac_tts_ctx *mac_tts_init(void);
void         mac_tts_shutdown(mac_tts_ctx *ctx);

/* Word-range callback. Invoked just before each word is spoken.
 * start/length are in UTF-16 code units of the string being synthesised
 * (AVSpeech reports an NSRange). A NULL callback disables word notifications. */
typedef void (*mac_tts_word_cb)(void *user_data,
                                unsigned int start, unsigned int length);

/* Speak the given UTF-8 text with the given options (NULL = defaults).
 * If word_cb is non-NULL it is called for each spoken word (live highlight).
 * Blocks until playback finishes. Returns 0 on success, non-zero on failure. */
int          mac_tts_speak(mac_tts_ctx *ctx, const char *utf8_text,
                           const mac_tts_options *opts,
                           mac_tts_word_cb word_cb, void *word_cb_user);

/* Pause / resume the current utterance at a word boundary. Safe to call from
 * another thread while mac_tts_speak() is blocked waiting. No-op if not
 * currently speaking / not paused. */
void         mac_tts_pause(mac_tts_ctx *ctx);
void         mac_tts_resume(mac_tts_ctx *ctx);

/* Cancel the current utterance immediately so a blocked mac_tts_speak()
 * returns. Used by the element's unlock() to break out of the synchronous
 * wait during a state change / stop (does NOT tear down the context). */
void         mac_tts_cancel(mac_tts_ctx *ctx);

#ifdef __cplusplus
}
#endif

#endif /* GSTMACTTSSINK_MACOS_H */
