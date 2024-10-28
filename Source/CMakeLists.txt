# CMake file for DISTRHO Plugins #
# ------------------------------ #

dpf_add_plugin(d_arpeggiator_plugin
  TARGETS jack lv2 vst2 vst3 clap
  FILES_DSP
      ArpeggiatorPlugin.cpp)


target_include_directories(
  d_midiThrough PUBLIC ".")
