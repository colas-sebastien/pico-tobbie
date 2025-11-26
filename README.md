![Tobbie](img/tobbie.jpg)
# pico-tobbie
Control Tobbie robot with a bluetooth gamepad thanks to Raspberry Pico 2 W

Code should propably work with Pico W

# Hardware
## Prepare Tobbie
Here is how to update the hardware to control Tobbie with a Raspberry Pico 2
- Remove the head
![Tobbie Headless](img/tobbie-headless.jpg)
- Cut battery and motor cables and a cordon JST ZH 1,5 mm
![Cables](img/cables.jpg)
- replace motor connectors by JST ZH 1,5 mm cables
![Head cables patch](img/tobbie-headless-patch-1.jpg)
- reuse the red motor connector on the battery
![Head battery patch](img/tobbie-headless-patch-2.jpg)
- add a power on button on the top of the head
![Tobbie Top](img/top.jpg)
![Tobbie Top Switch](img/top-button-1.jpg)
![Tobbie Top Switch](img/top-button-2.jpg)

## Raspberry Pico 2W + Motor SHIM
Here is the module that will be used to control the motors:
https://shop.pimoroni.com/products/motor-shim-for-pico
![Pico 2W](img/pico2w-2.jpg)
![Pico 2W](img/pico2w-3.jpg)
![Pico 2W](img/pico2w-4.jpg)


## PCB remplacement
![Electronic Schema](img/schema.jpg)
![Tobbie](img/pcb-1.jpg)
![Tobbie](img/pcb-2.jpg)
![Tobbie](img/pcb-3.jpg)
![Tobbie](img/pico2w-1.jpg)
![Tobbie](img/head.jpg)

## Connect everything
![Tobbie Final 1](img/tobbie-final-1.jpg)
![Tobbie Final 2](img/tobbie-final-2.jpg)

# Software

## Install dependencies

```
mkdir projects
cd projects
git clone https://github.com/raspberrypi/pico-sdk
cd pico-sdk
git submodule init
git submodule update
cd ..
git clone https://github.com/pimoroni/pimoroni-pico
git clone https://github.com/ricardoquesada/bluepad32
```

## Set up environment variables
```
export PICO_SDK_PATH=`pwd`/pico-sdk
export PIMORONI_PICO_FETCH_FROM_GIT=`pwd`/pimoroni-pico
export BLUEPAD32_ROOT=`pwd`/bluepad32
```
## Build the program

```
git clone https://github.com/colas-sebastien/pico-tobbie.git
cd pico-tobbie
mkdir build
cd build
cmake ..
make -j
```

Next step is to copy `pico-tobbie.uf2` on you pico by pressing the button and connecting it via USB
# References

- [Tobbie (by Velleman)](https://www.velleman.eu/products/view/?id=449416)
- [Motor SHIM for Pico (by Pimori)](https://shop.pimoroni.com/products/motor-shim-for-pico)
- [Control Motor SHIM code (by Pimori)](https://github.com/pimoroni/pimoroni-pico/tree/main/micropython/examples/pico_motor_shim)
- [Boiler Plate for pico (by Pimori)](https://github.com/pimoroni/pico-boilerplate)
- [BluePad32 for Pico (by Ricardo Quesada)](https://github.com/ricardoquesada/bluepad32/tree/main/examples/pico_w)

