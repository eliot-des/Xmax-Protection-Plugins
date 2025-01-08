# Xmax Protection Plugins : Audio plugins for mechanical overload protection of loudspeakers.
---
Eliot Deschang & Florian Marie 2025, IMDEA Master students.

This repository gathers three different plugins implemented as part of a Master 2 project scheduled over three months. The aim of the project was to develop algorithms to prevent mechanical overload of loudspeakers. Mechanical overload is prevented by limiting the membrane excursion to a certain threshold, referred to as $X_{\text{max}}$.  

During the course of this research, three plugins were developed to evaluate the effectiveness of these algorithms on a loudspeaker. The plugins were fully coded in C++ using the JUCE framework, after implementing and testing the algorithms in Python.


**Disclaimer:**  The GUI style is the one seen on this [repository](https://github.com/TheAudioProgrammer/getting-started-book). Also, as a self-taugh C++ coder, some of the code could certainly be better written, both in terms of clarity and optimisation.

## Notes on plugins design and performances
---
For detailed information about the algorithms design, performances on various test signals, and more, please refer to the [project report](https://github.com/eliot-des/Xmax-Protection-Plugins/blob/main/Deschang_Marie_Mechanical_protection_algorithms_for_loudspeakers.pdf) provided in this repository.

## Build
---
Since plugin formats differ depending on the operating system (Windows, macOS ARM/Intel, or Linux), prebuilt versions in VST/VST3/AAX formats are not provided in this repository, except for Windows, as it is the only operating system available on my machine.

To build these plugins from scratch, follow these steps:

1. Install the [JUCE Framework](https://juce.com/) on your machine.
2. Open the **Projucer** application.
3. Click on **File** in the top-left menu bar and select **Open...**.
4. Choose `XmaxLimiter.jucer` if you want to build the XmaxLimiter plugin.
5. Add the appropriate source code files associated with the plugin to the Projucer project.
6. Compile the plugin using your preferred IDE or build system.

## XmaxFeedback
---
![XmaxFeedback plugin image](https://github.com/eliot-des/Xmax-Protection-Plugins/blob/main/readme/XmaxFeedback.png)

## XmaxLimiter
---
![XmaxLimiter plugin image](https://github.com/eliot-des/Xmax-Protection-Plugins/blob/main/readme/XmaxLimiter.png)

## XmaxLowshelf
---
![XmaxLowShelf plugin image](https://github.com/eliot-des/Xmax-Protection-Plugins/blob/main/readme/XmaxLowShelf.png)
