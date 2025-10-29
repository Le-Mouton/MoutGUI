# MoutGUI

A lightweight C++ / OpenGL graphical interface framework for real-time scientific visualization.

This project is part of my ongoing learning in **C++** programming and **OpenGL** graphics development.  
MoutGUI is intentionally built from scratch (no high-level UI engines) to:

- Understand low-level rendering pipelines
- Manage GPU buffers and shaders directly
- Implement event propagation and UI interactions manually
- Learn how to design reusable and modular components in C++

I actively improve the framework **as my needs evolve** (e.g., for simulation tools, signal processing dashboards, or real-time visualization).  
I am open to feedback, suggestions, and constructive code review.  
If you notice design issues, performance inefficiencies, or better architectural approaches — feel free to propose changes or open discussions.

---

## Features

| Component | Description |
|---------|-------------|
| Sliders | Real-time control of numerical parameters |
| Buttons | Toggle states, actions, pause, etc. |
| Draggable Menus | Movable UI panels that contain other widgets |
| LinePlot | Dynamic time-series plots supporting continuous updating |
| ScatterPlot | Point cloud visualization with optional regression lines |
| HeatmapPlot | 2D scalar field visualization (heatmaps, spectrograms) |
| Interactive Resizing | Plots and menus can be resized using the mouse |
| TrueType Text Rendering | Scalable text with labels and titles |
| Event Propagation System | Mouse events are dispatched based on priority and focus |

---

## Directory Structure
```
src/
│── main.cpp            → Example application
│── item.hpp            → Base class for UI components
│── callback.hpp        → Central mouse/interaction manager
│── menu.hpp            → Movable and resizable UI panels
│── button.hpp          → UI buttons
│── slider.hpp          → UI sliders
│── text.hpp            → TrueType font rendering
│── plot.hpp            → Generic plot (grid, axes, labels)
│── lineplot.hpp        → Dynamic line plotting
│── scatter.hpp         → Scatter + regression plotting
│── heatmap.hpp         → Heatmap and spectrogram plotting
│── shader.hpp          → GLSL shader management
└── ttf-reader.hpp      → Lightweight TrueType font loader
```

---

## Dependencies

| Library | Purpose | Installation |
|--------|---------|---------------|
| GLFW | Window + mouse/keyboard handling | `brew install glfw` / `apt install glfw` |
| GLAD | OpenGL loader | Generated from https://glad.dav1d.de |
| OpenGL 3.3+ | GPU rendering | Included on Linux / Windows / macOS |
| glm | Vector and matrix math | `brew install glm` / `apt install libglm-dev` |

---

## Author

Mathys “Mout” Herbreteau
Aerospace Engineering • Scientific Visualization • Real-Time Simulation

