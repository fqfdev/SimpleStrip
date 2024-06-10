# SimpleStrip DAW Plugin

This repository provides the source code for a simple DAW plugin, implemented using C++ and JUCE. The plugin features a basic channel strip with two audio processors connected in series: a low-pass filter and a gain trim. This can serve as a starting point for building more complex audio processing plugins.

## Installation

Create a default JUCE plugin project and replace the generated source files with the ones provided in this repository.

### Prerequisites

- [JUCE](https://juce.com/get-juce) (latest version)
- A C++ development environment (e.g., Visual Studio, Xcode)

### Steps

1. **Create a new JUCE plugin project**:
   - Open the JUCE Projucer application.
   - Create a new project and choose the "Audio Plug-In" template.
   - Configure your project settings (name, company, etc.) and save it.

2. **Replace source files**:
   - Navigate to your project's `Source` folder.
   - Copy all the source files from this repository and paste them into the `Source` folder of your JUCE project, replacing the existing files.

3. **Build the project**:
   - Open your JUCE project in your chosen IDE (e.g., Visual Studio, Xcode).
   - Build the project.
   - Import "SimpleStrip.vst3" into your DAW and test.
   

## Usage

Once you have built the project, you can load the plugin in your DAW (Digital Audio Workstation) and use the following controls:

- **Low-Pass Filter**:
  - Adjust the cutoff frequency to control the point where the filter starts attenuating high frequencies.
  - Modify the resonance to increase the emphasis at the cutoff frequency.
  
- **Gain Trim**:
  - Use the gain control to adjust the output level of the audio signal.

## License

This project is licensed under the [MIT License](LICENSE). You are free to use, modify, and distribute this code.
