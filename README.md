# nRF9151-dk-mender

This is an Mender-MCU integration sample for [nRF9151 DK](https://www.nordicsemi.com/Products/Development-hardware/nRF9151-DK).

This project is based on [nRF Connect](https://github.com/nrfconnect) and hardware from [Nordic Semiconductor](https://www.nordicsemi.com/)

The project is not intended to be a complete reference design for a commercial product, but rather a source of inspiration.

## Hardware

Hardware used in this project:

* [nRF9151 DK](https://www.nordicsemi.com/Products/Development-hardware/nRF9151-DK)

You also need:

* USB-C cable
* SIM card with data plan

## Prerequisites

* Install [nRF Util](https://docs.nordicsemi.com/bundle/nrfutil/page/guides/installing.html.)

## Getting Started

Before getting started, make sure you have a proper nRF Connect development environment.

### Install nRF Connect SDK

```
nrfutil install sdk-manager
```

### Install v3.2.1 SDK

```
nrfutil sdk-manager install v3.2.1
```

### Start the nRF Connect SDK toolchain shell

Use `nrfutil` to launch a shell with the correct nRF Connect SDK toolchain
environment:

```bash
nrfutil sdk-manager toolchain launch --ncs-version v3.2.1 --shell
```

If the command succeeds, your shell prompt will change to something like:

```bash
(v3.2.1) [user@host ~]$
```

All remaining commands in this guide should be run inside that shell.

### Set up workspace

Create a new workspace and enter it:

```bash
mkdir -p ~/src/nrf9151-dk-mender-workspace && cd ~/src/nrf9151-dk-mender-workspace
```

Initialize the workspace:

```bash
west init -m https://github.com/id8-engineering/nrf9151-dk-mender --mr main .
```

Change into the project directory:

```bash
cd nrf9151-dk-mender
```

Fetch and check out sources:

```bash
west update
```

### Build the application

Build the firmware:

```bash
west build -p always -b nrf9151dk/nrf9151 app
```

### Flash firmware

Flash the firmware:

```bash
west flash
```

### Access console

```
minicom -D /dev/ttyACM0 -b 115200 # Change /dev/ttyACM0 to your actual device
```
