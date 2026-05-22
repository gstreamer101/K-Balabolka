#include <gst/gst.h>
#include <gst/base/gstbasesink.h>
#include <string.h>

#include "gstmacttssink_macos.h"

GST_DEBUG_CATEGORY_STATIC (gst_mac_tts_sink_debug);
#define GST_CAT_DEFAULT gst_mac_tts_sink_debug

#define GST_TYPE_MAC_TTS_SINK (gst_mac_tts_sink_get_type ())
G_DECLARE_FINAL_TYPE (GstMacTtsSink, gst_mac_tts_sink, GST, MAC_TTS_SINK, GstBaseSink)

struct _GstMacTtsSink
{
  GstBaseSink parent;
  mac_tts_ctx *tts;
};

G_DEFINE_TYPE (GstMacTtsSink, gst_mac_tts_sink, GST_TYPE_BASE_SINK)

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE (
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("text/x-raw, format=(string)utf8")
);

static gboolean
gst_mac_tts_sink_start (GstBaseSink * sink)
{
  GstMacTtsSink *self = GST_MAC_TTS_SINK (sink);
  GST_DEBUG_OBJECT (self, "start()");

  self->tts = mac_tts_init ();
  if (!self->tts) {
    GST_ERROR_OBJECT (self, "mac_tts_init() failed");
    return FALSE;
  }
  return TRUE;
}

static gboolean
gst_mac_tts_sink_stop (GstBaseSink * sink)
{
  GstMacTtsSink *self = GST_MAC_TTS_SINK (sink);
  GST_DEBUG_OBJECT (self, "stop()");

  if (self->tts) {
    mac_tts_shutdown (self->tts);
    self->tts = NULL;
  }
  return TRUE;
}

static GstFlowReturn
gst_mac_tts_sink_render (GstBaseSink * sink, GstBuffer * buffer)
{
  GstMacTtsSink *self = GST_MAC_TTS_SINK (sink);
  GstMapInfo map;

  if (!gst_buffer_map (buffer, &map, GST_MAP_READ)) {
    GST_ERROR_OBJECT (self, "failed to map buffer");
    return GST_FLOW_ERROR;
  }

  gchar *text = g_strndup ((const gchar *) map.data, map.size);
  /* AVSpeechSynthesizer는 빈/공백 utterance에 완료 콜백을 안 줘서 동기 대기가 무한정 됨 */
  g_strstrip (text);

  if (text[0] != '\0') {
    g_print ("[macttssink] speaking (%zu bytes): %s\n", strlen (text), text);
    if (self->tts && mac_tts_speak (self->tts, text) != 0) {
      GST_WARNING_OBJECT (self, "mac_tts_speak() failed for: %s", text);
    }
  } else {
    GST_DEBUG_OBJECT (self, "empty text buffer, skipping");
  }

  g_free (text);
  gst_buffer_unmap (buffer, &map);
  return GST_FLOW_OK;
}

static void
gst_mac_tts_sink_class_init (GstMacTtsSinkClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstBaseSinkClass *base_sink_class = GST_BASE_SINK_CLASS (klass);

  gst_element_class_set_static_metadata (element_class,
      "macOS TTS element",
      "Sink/Text",
      "Sink a text buffer and synthesise speech via macOS AVSpeechSynthesizer",
      "K-Balabolka <noreply@example.com>");

  gst_element_class_add_static_pad_template (element_class, &sink_template);

  base_sink_class->start  = GST_DEBUG_FUNCPTR (gst_mac_tts_sink_start);
  base_sink_class->stop   = GST_DEBUG_FUNCPTR (gst_mac_tts_sink_stop);
  base_sink_class->render = GST_DEBUG_FUNCPTR (gst_mac_tts_sink_render);
}

static void
gst_mac_tts_sink_init (GstMacTtsSink * self)
{
  self->tts = NULL;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_mac_tts_sink_debug, "macttssink", 0,
      "macOS TTS sink (C, AVSpeechSynthesizer)");
  return gst_element_register (plugin, "macttssink", GST_RANK_NONE,
      GST_TYPE_MAC_TTS_SINK);
}

#ifndef PACKAGE
#define PACKAGE "gst-macttssink-c"
#endif

GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    macttssink,
    "macOS TTS sink (C, AVSpeechSynthesizer)",
    plugin_init,
    "0.2.0",
    "MIT/Apache-2.0",
    PACKAGE,
    "https://example.com/k-balabolka"
)
