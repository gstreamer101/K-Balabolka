project('gst-ttssink-c', 'c',
  version : '0.1.0',
  default_options : [
    'warning_level=2',
    'c_std=c11',
    'buildtype=debug',
  ],
)

gst_dep      = dependency('gstreamer-1.0',      version : '>= 1.20')
gst_base_dep = dependency('gstreamer-base-1.0', version : '>= 1.20')

plugin_sources = [
  'gstttssink.c',
]

shared_library('gstttssink',
  plugin_sources,
  dependencies : [gst_dep, gst_base_dep],
  install      : false,
  name_prefix  : '',
)
