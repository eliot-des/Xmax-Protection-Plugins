# Xmax Protection Plugins : Audio plugins for mechanical overload protection of loudspeakers.
---
[![CC BY-NC-SA 4.0][cc-by-nc-sa-shield]][cc-by-nc-sa]


Eliot Deschang & Florian Marie 2025, IMDEA Master students, Le Mans University.

This repository gathers three different plugins implemented as part of a Master 2 project scheduled over three months, supervised by [Antonin Novak](https://ant-novak.com/). The aim of the project was to develop algorithms to prevent mechanical overload of loudspeakers. Mechanical overload is prevented by limiting the membrane excursion to a certain threshold, referred to as $X_{\text{max}}$.  

During the course of this research, three plugins were developed to evaluate the effectiveness of these algorithms on a loudspeaker. The plugins were fully coded in C++ using the JUCE framework, after implementing and testing the algorithms in Python.

**Disclaimer**: As a self-taugh C++ coder, some of the code could certainly be better written, both in terms of clarity and optimisation.

## Notes on plugins design and performances
---
For detailed information about the algorithms design, performances on various test signals, and more, please refer to the [project report](https://github.com/eliot-des/Xmax-Protection-Plugins/blob/main/Report_Mechanical_protection_algorithms_for_loudspeakers.pdf) provided in this repository.

## Build
---
Since plugin formats differ depending on the operating system (Windows, macOS ARM/Intel, or Linux), prebuilt versions in VST/VST3/AAX formats are not provided in this repository, except for Windows (check release section on the right), as it is the only operating system available on my machine.

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


## Credit
---

- **GUI Design:** [Matthijs Hollemans](https://github.com/hollance), who created this [repository](https://github.com/TheAudioProgrammer/getting-started-book), from which the majority of the GUI components are derived.  
- **Lato Font:** Designed by Łukasz Dziedzic.
--- 

## Licences
---
This work is licensed under a
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License][cc-by-nc-sa].

[![CC BY-NC-SA 4.0][cc-by-nc-sa-image]][cc-by-nc-sa]

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-image]: https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png
[cc-by-nc-sa-shield]: https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg

For a change in licence, and/or if one/some algorithms are intented to be used by a company, please contact the authors.
