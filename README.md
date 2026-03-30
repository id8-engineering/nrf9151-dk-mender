# nRF9151-dk-mender

This is an Mender-MCU integration sample for nRF9151 DK.

This project is based on [nRF Connect](https://github.com/nrfconnect) and hardware from [Nordic Semiconductor](https://www.nordicsemi.com/)

The project is not intended to be a complete reference design for a commercial product, but rather a source of inspiration.

## Hardware

Hardware used in this project:

* [nRF9151 DK](https://www.nordicsemi.com/Products/Development-hardware/nRF9151-DK)

You also need:

* USB-C cable
* SIM card with data plan

## Getting Started

### Prerequisites

Before getting started, make sure you have a proper nRF Connect development
environment:

#### Install nrfutil

Download the [nrfutil](https://www.nordicsemi.com/Products/Development-tools/nRF-Util) binary for your platform from Nordic Semiconductor.

On Linux, install it like this:

```bash
chmod +x nrfutil
mkdir -p ~/.local/bin
mv nrfutil ~/.local/bin/
export PATH="$HOME/.local/bin:$PATH"
```

Verify the installation:

```bash
nrfutil --version
nrfutil list
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

### Set up workspace and Python tools

Create a new workspace and enter it:

```bash
mkdir -p ~/src/nrf9151-dk-mender-workspace && cd ~/src/nrf9151-dk-mender-workspace
```

Create a Python virtual environment:

```bash
python3 -m venv .venv
```

Activate the Python virtual environment:

```bash
source .venv/bin/activate
```

Install `west`:

```bash
pip install --upgrade pip
pip install west
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

Install Python dependencies:

```bash
west packages pip --install
```

### Build the application

Build the firmware:

```bash
west build -p always -b nrf9151dk/nrf9151 app
```

### Flash firmware

Flash the firmware with J-Link:

```bash
west flash -r jlink
```
