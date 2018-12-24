#!/bin/bash

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


# Сheck that previous command was executed successfully
checking_error () {
    if [ $1 -ne 0 ]
    then
        echo "command executing error"
        exit 2
    fi
}

show_gw_id() {
    echo "**************** Gateway ID ****************"
    echo "*                                          *"
    echo "*             gw$(cat /sys/class/net/eth0/address | sed s/\://g)               *"
    echo "*                                          *"
    echo "********************************************"
}

show_help () {
    echo "Usage: iqot_gw.sh [OPTION]... [PARAMETER]..."
    echo -e "install, start, stop, remove, id\n"
    echo "[OPTION] parameters:"
    echo "      install         install gateway software. Specify path to the whitelist.txt file using [PARAMETER]."
    echo "                      Two docker images will be pulled: plviqot/iqot_gw_core_app_arm and plviqot/iqot_gw_ble_sample_arm"
    echo "                      Please, run script with sudo option."
    echo "      start           start IQoT docker containers. It will bring up the following images: plviqot/iqot_gw_core_app_arm and plviqot/iqot_gw_ble_sample_arm"
    echo "      stop            stop running IQoT dockers containers"
    echo "      id              show Gateway ID"
    echo -e "      remove          remove IQoT docker containers and images.\n"
}

install () {
    if [[ -z "$1" ]]
    then
        echo "'install' option - missed path to the 'whitelist.txt' file"
        exit 1
    elif [[ "$1" != *"whitelist.txt"* ]]
    then
        echo "'install' option got unexpected parameter - $1"
        exit 1
    fi

    docker --version
    if [ $? -ne 0 ]
    then
            echo "Installing docker..."
            curl -sSL https://get.docker.com | sh
            checking_error $?
    fi

    echo "Install supervisor..."
    apt-get update && apt-get install supervisor
    checking_error $?

    docker pull plviqot/iqot_gw_core_app_arm
    checking_error $?
    docker pull plviqot/iqot_gw_ble_sample_arm
    checking_error $?

    echo "writing supervisor config files"

    ALL_RUNNING="while ( ! docker stats --no-stream > /dev/null ) || [ -z \"\`service --status-all | grep '[+].*bluetooth'\`\" ]; do sleep 1; echo \$(date) \"Waiting for docker and bluetooth services\"; done"
    BL_STOPPED="while [ -z \"\`service --status-all | grep '[-].*bluetooth'\`\" ]; do sleep 1; echo \$(date) \"Waiting for bluetooth service\"; done"

    echo -e "[program:iqot_coreapp]\n"\
"directory=/home/pi/\n"\
"command=bash -c '$ALL_RUNNING && docker run -p 6379:6379 --mac-address $(cat /sys/class/net/eth0/address) --rm --name iqot_coreapp plviqot/iqot_gw_core_app_arm'\n"\
"stdout_logfile=/var/log/supervisor/iqot_coreapp.log\n"\
"redirect_stderr=true\n"\
"autorestart=true\n"\
"killasgroup=true\n"\
"stopasgroup=true\n"\
"priority=100\n"\
"\n"\
"[program:iqot_driver]\n"\
"directory=/home/pi/\n"\
"command=bash -c '$ALL_RUNNING && service bluetooth stop && $BL_STOPPED && docker run --rm -v $1:/etc/iqot/config/ble/whitelist.txt --privileged --network host --name iqot_driver plviqot/iqot_gw_ble_sample_arm'\n"\
"stdout_logfile=/var/log/supervisor/iqot_driver.log\n"\
"redirect_stderr=true\n"\
"startretries=15\n"\
"autorestart=true\n"\
"killasgroup=true\n"\
"priority=200\n"\
"stopasgroup=true\n\n"\
"[unix_http_server]\n"\
"file=/var/run/supervisor.sock\n"\
"chmod=0770\n"\
"chown=root:supervisor"\
"\n\n"\
"[supervisorctl]\n"\
"serverurl=/var/tmp/supervisord.sock\n" > /etc/supervisor/conf.d/iqot.conf

    checking_error $?

    show_gw_id

    supervisorctl reread
    checking_error $?
    supervisorctl update
    checking_error $?

    echo "IQoT Services already installed."
    echo "Please reboot your machine"

}

start () {
    echo -e "Going to start docker images\n"
    echo -e "Before running, please add gateway ID to the IQoT web application\n"
    show_gw_id
    echo "Press 'Enter' to continue after entering ID to the IQoT web application"
    read
    echo "Starting gateway core application"
    service bluetooth start
    supervisorctl start iqot_coreapp iqot_driver
}

stop () {
    echo -e "Stopping docker images..."
    supervisorctl stop iqot_driver iqot_coreapp
    docker rm -f iqot_driver iqot_coreapp
}

remove () {
    stop
    echo "Removing docker images..."
    docker rm -f iqot_driver iqot_coreapp
    docker rmi -f plviqot/iqot_gw_ble_sample_arm plviqot/iqot_gw_core_app_arm
    rm /etc/supervisor/conf.d/iqot.conf
}

if [ $# -eq 0 ]
then
    show_help
    exit 0
fi

for i in $@
do
    if [ $i = "-h" ] || [ $i = "--help" ]
    then
        show_help
        exit 0
    elif [ $i = "id" ]
    then
        show_gw_id
        exit 0
    fi
done

case "$1" in
    install)
        install $2
        ;;
    start)
        start
        ;;
    stop)
        stop
        ;;
    id)
        show_gw_id
        ;;
    remove)
        remove
        ;;
esac
