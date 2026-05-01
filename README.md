# Yocto Build for SPIRIT Phone

This repo aims to act as a reference build setup for the
[SPIRIT phone](https://github.com/V3lectronics/SPIRIT).
It uses [`kas`](https://github.com/siemens/kas) to simplify setup and building.

## Motivation

Most discussion around Operating System choice for a project quickly drifts into
the **userland** (desktop environments, init systems, apps).
That is important but the foundation is always the same:
- a working **Linux kernel**
- the right **device tree** describing the hardware
- firmware, bootloader, and low-level integration

This repo exists to focus on that **base system**.

- **Reproducibility** - same kernel/device tree build every time, across hosts.
- **Portability** - the kernel and DTBs built here can be used by other distributions
  (Mobian, Ubuntu Touch, Arch ARM, etc.).
- **Customizability** - we can maintain multiple targets,
  tweak device trees, or patch the kernel for SPIRIT’s custom hardware.
- **Reference BSP** - this layer documents exactly what’s needed to boot
  the SPIRIT phone, making it easier for other projects to bring up support for SPIRIT.
- **Testing ground** - a minimal base to validate peripherals,
  benchmark performance and measure battery life.

In other words this is not a "phone OS" but a **hardware enablement layer**
that ensures Linux distributions can work at all on SPIRIT.

## Requirements
- Linux host (or WSL2 on Windows / Docker Desktop on macOS)
- Docker or Podman installed
- ~50–100 GB free disk space, a moderate amount of CPU/RAM (first build is heavy!)
- [`kas`](https://kas.readthedocs.io/en/latest/userguide/getting-started.html)

## Build

Clone this repository, then go **one directory up** before building.
(`kas` will create the actual build directory there and fetch other layers automatically.)

To configure the image run:
```sh
kas-container menu meta-spirit/Kconfig
```

After selecting the desired features select `Build` in the menu.

The build can be ran/resumed by simply running

```sh
kas-container build
```

This by default will use the `.config.yaml`. Alternatively one can parse
a different file to it.

```sh
kas-container build <saved_config.yml>
```

### Resource-usage

The first build can take hours on a personal computer/laptop.

To limit CPU usage (example: 4 cores):

```sh
export BB_NUMBER_THREADS=4
export PARALLEL_MAKE="-j4"
kas-container build
```

### Troubleshooting

Most errors aren’t scary. If a recipe fails, just clean it and rebuild.

#### Example error:

```log
ERROR: Task (.../gcc_13.4.bb:do_compile) failed with exit code '1'
```

Fix:

```sh
# open a shell inside the build container
kas-container shell

# clean the recipe
bitbake -c cleanall gcc

# exit and rebuild
exit
kas-container build
```

When in doubt refer to
[Yocto Project Documentation](https://docs.yoctoproject.org/5.0.12/singleindex.html)
or open an issue.

## Running

### QEMU - `qemuarm64`

> **Note**
> QEMU is run *inside* the build container to ensure it uses the
> exact `qemu-system-native` binary and libraries that Yocto built.
> This avoids host dependency issues and guarantees a reproducible
> runtime environment. The built artifacts can still be ran with reqular
> `qemu-system-aarch64`.

#### Serial

```sh
kas-container shell

# From inside the container
runqemu slirp serialstdio
```
#### Graphical

```sh
kas-container --runtime-args "-p 5900:5900" shell

# From inside the container
runqemu slirp publicvnc

# Now on your host
vncviewer localhost:5900
```

This can be combined with `serialstdio` to get early boot messages.

### CM5 - `spirit-phone-cm5`

#### eMMC

To flash the eMMC on the IO Board the
[`rpiboot`](https://github.com/raspberrypi/usbboot?tab=readme-ov-file#troubleshooting)
utility needs to be used to put the eMMC into flash mode.

1. A `EMMC-DISABLE / nRPIBOOT (BCM2712 GPIO 20)` pin must be lowered by connecting a jumper/F-F cable.
1. Connect the device USB-C port to your host.
1. Run [`rpiboot`](https://github.com/raspberrypi/usbboot) to put eMMC into flash mode.
    - To ease up development a [container](./scripts/rpiboot/Dockerfile) has been prepared for this step.
        ```sh
        # build the container
        $ docker build -t rpiboot meta-spirit/scripts/rpiboot

        # run it with --privileged and passing the peripherals. Sometimes needs to be ran twice. Below is an example correct output
        $ docker run --rm -it --init \
            --privileged \
            --device /dev:/dev \
            rpiboot

        RPIBOOT: build-date 2025/09/13 pkg-version local 466e26dc

        Please fit the EMMC_DISABLE / nRPIBOOT jumper before connecting the power and USB cables to the target device.
        If the device fails to connect then please see https://rpltd.co/rpiboot for debugging tips.

        Waiting for BCM2835/6/7/2711/2712...

        Directory not specified - trying default /usr/share/rpiboot/mass-storage-gadget64/
        Second stage boot server
        File read: mcb.bin
        File read: memsys00.bin
        File read: memsys01.bin
        File read: memsys02.bin
        File read: memsys03.bin
        File read: bootmain
        Loading: /usr/share/rpiboot/mass-storage-gadget64//config.txt
        File read: config.txt
        Loading: /usr/share/rpiboot/mass-storage-gadget64//boot.img
        File read: boot.img
        Second stage boot server done

        # a new device should appear in your lsblk/dmesg. Make sure it's unmounted before proceeding
        $ umount /dev/sdX
        ```
1. Copy the image to the eMMC.
    ```sh
    $ bmaptool copy build/tmp-glibc/deploy/images/spirit-phone-cm5/core-image-minimal-spirit-phone-cm5.rootfs.wic.bz2 /dev/sdX
    bmaptool: info: discovered bmap file 'build/tmp/deploy/images/raspberrypi5/core-image-minimal-raspberrypi5.rootfs.wic.bmap'
    bmaptool: info: discovered bmap file 'build/tmp-glibc/deploy/images/raspberrypi5/core-image-minimal-raspberrypi5.rootfs.wic.bmap'
    bmaptool: info: block map format version 2.0
    bmaptool: info: 43707 blocks of size 4096 (170.7 MiB), mapped 16919 blocks (66.1 MiB or 38.7%)
    bmaptool: info: copying image 'core-image-minimal-raspberrypi5.rootfs.wic.bz2' to block device '/dev/sda' using bmap file 'core-image-minimal-raspberrypi5.rootfs.wic.bmap'
    bmaptool: info: 100% copied
    bmaptool: info: synchronizing '/dev/sda'
    bmaptool: info: copying time: 3.0s, copying speed 22.0 MiB/sec
    ```
1. Remove the jumper from the `EMMC-DISABLE / nRPIBOOT (BCM2712 GPIO 20)`.
1. Power cycle the device.

#### SD card

1. Plug your microSD card into your host. Make sure it's unmounted before proceeding.
1. Copy the image to the microSD.
    ```sh
    $ bmaptool copy build/tmp-glibc/deploy/images/spirit-phone-cm5/core-image-minimal-spirit-phone-cm5.rootfs.wic.bz2 /dev/sdX
    bmaptool: info: discovered bmap file 'build/tmp-glibc/deploy/images/raspberrypi5/core-image-minimal-raspberrypi5.rootfs.wic.bmap'
    bmaptool: info: block map format version 2.0
    bmaptool: info: 43707 blocks of size 4096 (170.7 MiB), mapped 16919 blocks (66.1 MiB or 38.7%)
    bmaptool: info: copying image 'core-image-minimal-raspberrypi5.rootfs.wic.bz2' to block device '/dev/sda' using bmap file 'core-image-minimal-raspberrypi5.rootfs.wic.bmap'
    bmaptool: info: 100% copied
    bmaptool: info: synchronizing '/dev/sda'
    bmaptool: info: copying time: 3.0s, copying speed 22.0 MiB/sec
    ```
1. Plug the microSD into the device and power cycle it.
