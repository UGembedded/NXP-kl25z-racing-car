# NXP KL25Z Autonomous Racing Car

Project using the NXP FRDM-KL25Z microcontroller to control
an autonomous racing car using camera-based track detection, motor PWM
and servo steering.

## Project Overview

The system uses a line-scan camera to detect the position of the track.
The KL25Z samples the camera's analogue output using its ADC, processes
the captured image data and adjusts the car's steering and motor output.

The project demonstrates direct interaction with several KL25Z hardware
peripherals including GPIO, ADC, timers and PWM.

## Hardware

- NXP FRDM-KL25Z
- ARM Cortex-M0+ microcontroller
- Line-scan camera
- DC motor
- Steering servo
- Motor driver circuitry


## Key Features

- Camera signal acquisition
- ADC-based analogue sampling
- Timer-generated camera timing
- PWM motor speed control
- PWM servo steering control
- GPIO control of camera SI and CLK signals
- Direct register-level peripheral configuration

## Camera Interface

The line-scan camera uses three main signals:

- `SI` → PTD7
- `CLK` → PTE1
- Analogue output → PTD5 / ADC channel 6

The KL25Z generates the SI and CLK timing signals and samples the
camera output through the ADC.

The captured image is stored in a 128-element array for processing.

## Motor and Steering Control

PWM is generated using the KL25Z TPM peripherals.

The timer controls the PWM period while the channel value controls
the duty cycle.

For example, the motor output uses TPM1 and its PWM channel to vary
the motor drive level.

The steering servo is also controlled using a PWM pulse whose width
determines the steering position.

## KL25Z Peripherals Used

- GPIO — camera control signals
- ADC — camera analogue signal acquisition
- TPM0 — camera timing/delays
- TPM1 — PWM generation
- Pin multiplexing — connects physical pins to timer/peripheral functions

## Repository structure: 

```text
NXP-KL25Z-Racing-Car/
├── src/
│   ├── main.c                 # Camera reading and line-following logic
│   ├── motor.c                # Motor PWM and H-bridge control
│   ├── steering.c             # Steering servo configuration
│   └── uart.c                 # Serial communication and debug output
├── include/
│   ├── motor.h                # Motor function declarations
│   ├── steering.h             # Steering function declarations
│   └── uart.h                 # UART function declarations
├── assets/
│   ├── car-photo.jpg          # Photo of the assembled car

```


## Photos:
- <img width="1600" height="1204" alt="image" src="https://github.com/user-attachments/assets/864b03a5-d1a2-4d90-b4aa-d00908806cc2" />
<img width="1204" height="1600" alt="image" src="https://github.com/user-attachments/assets/80801ea7-eb5a-4e5d-bce3-6a396d12803a" />
<img width="1600" height="1204" alt="image" src="https://github.com/user-attachments/assets/902a872d-6caa-4864-8ff4-e9537550e7b8" />


