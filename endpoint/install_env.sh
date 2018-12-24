#!bin/bash/

#==============================================================================
#
# The file is part of IQoT project which release under Apache 2.0 license.
# See the License file in the root of project.
# _____________________________________________________________________________
# Copyright (c) 2018 PLVision sp. z o.o. All Rights Reserved.
#
# NOTICE: All information contained herein is, and remains the property of
# PLVision sp. z o.o. and its suppliers, if any. The intellectual and technical
# concepts contained herein are proprietary to PLVision sp. z o.o. and its
# suppliers, and are protected by trade secret or copyright law. Dissemination
# of this information or reproduction of this material is strictly forbidden
# unless prior written permission is obtained from PLVision.
#
#==============================================================================

echo 'export PATH="$PATH:$HOME/esp/xtensa-esp32-elf/bin"' >> ~/.profile
echo "export IDF_PATH=~/esp/esp-idf" >> ~/.profile

apt-get install gcc git wget make libncurses-dev flex bison gperf python python-pip python-setuptools python-serial python-cryptography python-future python-pyparsing

mkdir ~/esp && cd ~/esp

wget https://dl.espressif.com/dl/xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz
tar -xzf xtensa-esp32-elf-linux64-1.22.0-80-g6c4433a-5.2.0.tar.gz

git clone --recursive https://github.com/espressif/esp-idf.git
git pull origin master
git checkout v3.3-dev
git submodule update --init --recursive && cd -

pip install -r ~/esp/esp-idf/requirements.txt

echo "EXTRA_COMPONENT_DIRS=$(pwd)/components" >> Makefile

chown -R $1:$1 ~/esp/
