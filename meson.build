project(
  'jscolour', 'c',
  version: '1.0',
  license: 'GPL'
)

cc = meson.get_compiler('c')

executable(
  'main',
  sources: [
    'bspfile.c',
    'cmdlib.c',
    'entities.c',
    'ltface.c',
    'mathlib.c',
    'tex.c',
    'threads.c',
    'trace.c',
    'tyrlite.c',
    'tyrlog.c'
  ],
  dependencies: [
    cc.find_library('m', required : false)
  ],
  c_args: [
    '-Wno-unused'
  ]
)
