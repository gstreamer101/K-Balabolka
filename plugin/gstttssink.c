#include <gst/gst.h>
#include <gst/base/gstbasesink.h>
#include <string.h>

GST_DEBUG_CATEGORY_STATIC (gst_tts_sink_debug);
#define GST_CAT_DEFAULT gst_tts_sink_debug

#define GST_TYPE_TTS_SINK (gst_tts_sink_get_type ())
G_DECLARE_FINAL_TYPE (GstTtsSink, gst_tts_sink, GST, TTS_SINK, GstBaseSink)

struct _GstTtsSink
{
  GstBaseSink parent;
};

G_DEFINE_TYPE (GstTtsSink, gst_tts_sink, GST_TYPE_BASE_SINK)

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE (
    "sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("text/x-raw, format=(string)utf8")
);

static gboolean
gst_tts_sink_start (GstBaseSink * sink)
{
  GST_DEBUG_OBJECT (sink, "start()");
  return TRUE;
}

static gboolean
gst_tts_sink_stop (GstBaseSink * sink)
{
  GST_DEBUG_OBJECT (sink, "stop()");
  return TRUE;
}

static GstFlowReturn
gst_tts_sink_render (GstBaseSink * sink, GstBuffer * buffer)
{
  GstMapInfo map;

  if (!gst_buffer_map (buffer, &map, GST_MAP_READ)) {
    GST_ERROR_OBJECT (sink, "failed to map buffer");
    return GST_FLOW_ERROR;
  }

  gchar *text = g_strndup ((const gchar *) map.data, map.size);
  g_print ("[ttssink] received text (%zu bytes): %s\n", map.size, text);
  g_free (text);

  gst_buffer_unmap (buffer, &map);
  return GST_FLOW_OK;
}

static void
gst_tts_sink_class_init (GstTtsSinkClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstBaseSinkClass *base_sink_class = GST_BASE_SINK_CLASS (klass);

  gst_element_class_set_static_metadata (element_class,
      "TTS element",
      "Sink/Text",
      "Sink a text buffer and (eventually) synthesise speech via macOS APIs",
      "K-Balabolka <noreply@example.com>");

  gst_element_class_add_static_pad_template (element_class, &sink_template);

  base_sink_class->start  = GST_DEBUG_FUNCPTR (gst_tts_sink_start);
  base_sink_class->stop   = GST_DEBUG_FUNCPTR (gst_tts_sink_stop);
  base_sink_class->render = GST_DEBUG_FUNCPTR (gst_tts_sink_render);
}

static void
gst_tts_sink_init (GstTtsSink * self)
{
  (void) self;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_tts_sink_debug, "ttssink", 0,
      "TTS sink (C, Stage 1 skeleton)");
  return gst_element_register (plugin, "ttssink", GST_RANK_NONE,
      GST_TYPE_TTS_SINK);
}

#ifndef PACKAGE
#define PACKAGE "gst-ttssink-c"
#endif

GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    ttssink,
    "TTS sink (C version, Stage 1 skeleton)",
    plugin_init,
    "0.1.0",
    "MIT/Apache-2.0",
    PACKAGE,
    "https://example.com/k-balabolka"
)
