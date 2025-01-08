# Synchronized Data Acquisition System (SDAS)

## Overview

**Synchronized Data Acquisition System (SDAS)** is a versatile tool designed to enable precise and synchronized data recording from multiple sensors, including low-cost and less accurate devices. The core functionality is powered by the **TSA (Temporal Sample Alignment)** algorithm, which ensures temporal alignment of samples, delivering consistent and accurate data across various sensor types.

## Features

- **Temporal Sample Alignment (TSA):**
  - Automatically aligns sensor data in time, handling different sampling rates and delays.
- **Modularity:**
  - Easily add new sensors and configure inputs/outputs.
- **Scalability:**
  - Supports simultaneous data acquisition from multiple sensors.
- **Performance:**
  - Optimized for efficiency, requiring minimal hardware resources.

## Applications

SDAS is suitable for various use cases, such as:
- **Environmental Monitoring:** Collecting data from sensors measuring temperature, humidity, and pressure.
- **IoT Systems:** Integrating multiple sensors for smart home or industrial IoT applications.
- **Industrial Diagnostics:** Monitoring and diagnosing machinery in real-time.
- **Scientific Research:** Synchronizing data from diverse instruments for accurate analysis.

## Getting Started

### Prerequisites

- C++ and Python3 library, needs to update as simple package
    - Compatible hardware, such as: Intel i
    - Microcontrollers (e.g., Jetson AGX Xavier)

### Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/sdas.git
   cd sdas
