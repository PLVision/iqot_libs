# Emulator setup

The `emulator.sh` file can be used to install, start, stop and remove the IQoT emulator sample.

Show help: `emulator.sh`. After the command executes, you will see a help menu.

`-i string` or `--network-interface string` is [Network interface](https://en.wikipedia.org/wiki/Network_interface) (software and/or hardware) that is an interface between two pieces of equipment or protocol layers in a computer network . It can be used to create the IQoT Gateway and emulator IDs. This parameter is to be used with `sudo emulator.sh install` and `emulator.sh start` commands.

- install emulator: `emulator.sh install`. The script will install docker for your system and pull 2 docker images. If you do not have docker, please run this command with `sudo`, or install docker by yourself. Without the '-i' or '--network-interface' parameter, the default 'enp2s0' ubuntu network interface will be used.

- start emulator: `emulator.sh start`. Without the '-i' or '--network-interface' parameter, the default 'enp2s0' ubuntu network interface will be used.

- stop emulator: `emulator.sh stop`. 

- remove emulator images: `emulator.sh remove`.


