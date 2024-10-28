
# Arpeggiator Plugin

An advanced MIDI arpeggiator plugin built with the DISTRHO Plugin Framework (DPF). This plugin provides highly customizable arpeggiation, allowing musicians to shape MIDI notes with flexible speed, patterns, and octave control.

## Features

- **Adjustable Speed Control**: Set arpeggiator speed up to 693 BPM or enter custom BPM values for precise control.
- **Multiple Patterns**:
  - **Up**: Ascending through notes.
  - **Down**: Descending from the highest note.
  - **Repeat-Twice**: Each note plays twice before moving to the next.
  - **Ascend-Descend**: Cycles up then down.
  - **Descend-Ascend**: Cycles down then up.
  - **Repeat-Thrice**: Each note plays three times before moving to the next.
- **Octave Range**: Control the range of octaves for more harmonic variation.
- **Manual Stop Button**: Instantly stop all notes with a single button.

## Installation

### Prerequisites

- [CMake](https://cmake.org/download/) (for building the plugin)
- A DAW that supports LV2 plugins, such as Ardour
- **DISTRHO Plugin Framework (DPF)**: This can be set up either by cloning DPF as a Git submodule or by manually downloading it.

### Option 1: Clone with DPF as a Submodule

1. Clone this repository, including its submodules:
   ```bash
   git clone --recurse-submodules https://github.com/YourUsername/ArpeggiatorPlugin.git
   ```

   > If you’ve already cloned the repository without submodules, you can initialize the submodule separately:
   > ```bash
   > git submodule update --init --recursive
   > ```

2. Proceed with the [Build Instructions](#build-instructions).

### Option 2: Manual DPF Setup

1. Clone this repository:
   ```bash
   git clone https://github.com/YourUsername/ArpeggiatorPlugin.git
   cd ArpeggiatorPlugin
   ```

2. Download [DPF](https://github.com/DISTRHO/DPF) and place it in the `Libraries/DPF` directory:
   ```bash
   git clone https://github.com/DISTRHO/DPF.git Libraries/DPF
   ```

3. Proceed with the [Build Instructions](#build-instructions).

### Build Instructions

1. Create a `build` directory and configure the build with CMake:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

2. Build the plugin:
   ```bash
   make
   ```

3. Copy the resulting `.lv2` folder to your system’s LV2 directory (typically `~/.lv2/` on Linux):
   ```bash
   cp -r ArpeggiatorPlugin.lv2 ~/.lv2/
   ```

4. Launch your DAW (e.g., Ardour) and load the plugin from the MIDI Processor plugins.

## Usage

1. **Speed**: Adjust the arpeggiator speed with the slider or type in a custom BPM value.
2. **Pattern**: Choose from six unique arpeggiation patterns to shape the sound.
3. **Octave Range**: Set the octave span to add harmonic depth.
4. **Stop Button**: Instantly stops all arpeggiated notes.

## Customization

- **Exploring Patterns**: Try different patterns to achieve various rhythmic textures.
- **Custom BPM**: Enter specific BPM values to find the exact tempo needed for your track.

## Contributing

Contributions, issues, and feature requests are welcome! Feel free to check out the [issues page](https://github.com/YourUsername/ArpeggiatorPlugin/issues) if you’d like to suggest improvements or report bugs.

## License

This project is licensed under the ISC License. See the [LICENSE](./LICENSE) file for details.

## Acknowledgments

- Built with the DISTRHO Plugin Framework (DPF)
- Inspired by classic synthesizer arpeggiators

---

This README provides a clear setup guide, ensuring users can install and use the plugin seamlessly. Let me know if there’s anything else you’d like to add!
