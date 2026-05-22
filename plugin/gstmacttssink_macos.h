#ifndef GSTMACTTSSINK_MACOS_H
#define GSTMACTTSSINK_MACOS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle. Created by mac_tts_init(), destroyed by mac_tts_shutdown(). */
typedef struct mac_tts_ctx mac_tts_ctx;

mac_tts_ctx *mac_tts_init(void);
void         mac_tts_shutdown(mac_tts_ctx *ctx);

/* Speak the given UTF-8 text. Blocks until playback finishes.
 * Returns 0 on success, non-zero on failure. */
int          mac_tts_speak(mac_tts_ctx *ctx, const char *utf8_text);

#ifdef __cplusplus
}
#endif

#endif /* GSTMACTTSSINK_MACOS_H */
