#!bin/bash

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


IQOT_EMULATOR="iqot_emulator"
IQOT_COREAPP="iqot_coreapp"
DOCKER_WAS_INSTALLED=0
INTERFACE="enp2s0"

checking_error () {
	if [ $1 -ne 0 ]
	then
		echo "command executing error"
		exit 2
	fi
}

show_ids(){
    echo "**************** Gateway ID ****************"
    echo "*                                          *"
    echo "*              $GATEWAY_ID		   *"
    echo "*                                          *"
    echo "********************************************"
    echo ""
    echo "**************** Endpoint ID ****************"
    echo "*                                           *"
    echo "*              $ENDPOINT_ID               *"
    echo "*                                           *"
    echo "*********************************************"
}


show_help () {
	echo "Usage: emulator.sh [OPTION]... [PARAMETER]..."
	echo -e "install, start, stop and remove iqot emulator sample \n"
	echo "[OPTION] parameters:"
	echo "	install 		install tools for installing iqot iqot_emulator to host. Without '-i' or '--network-interface' parameter will be taken default 'enp2s0' ubuntu network interface." 
	echo "				Please, run script with this option by sudo."
	echo "	start 			start iqot emulator sample. Without '-i' or '--network-interface' parameter will be taken default 'enp2s0' ubuntu network interface"
	echo -e "				If emulator is not installed to host script will write error message.\n"
	echo "	stop 			stop iqot emulator sample. If emulator is not installed to host script will write error message"
	echo -e "	remove 			remove iqot emulator images.\n"
	echo "[PARAMETER] parameters:"
	echo "	-i, --network-interface string	 Network Interface for creating IQoT gateway and emulator IDs.This parameter used with 'install' and 'start"
}

install_docker() {
	apt-get update
	sudo apt-get install \
	    apt-transport-https \
	    ca-certificates \
	    curl \
	    software-properties-common
	curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo apt-key add -
	sudo add-apt-repository \
	   "deb [arch=amd64] https://download.docker.com/linux/ubuntu \
	   $(lsb_release -cs) \
	   stable"
	apt-get update
	apt-get install docker-ce

	groupadd docker > /dev/null 2>&1

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
	fi
done

case "$1" in
	install)

		if [ "$2" = "-i" ] || [ "$2" = "--network-interface" ]
		then
			INTERFACE="$3"
		fi

		ifconfig $INTERFACE > /dev/null 2>&1
		if [ $? -ne 0 ]
		then
			echo "invalid network Interface. Please, run script 'emulator.sh start' with '-i' or '--network-interface' option"
			exit 1
		fi

		GATEWAY_ID="gw$(cat /sys/class/net/$INTERFACE/address | sed s/\://g)"
		ENDPOINT_ID="ep$(cat /sys/class/net/$INTERFACE/address | sed s/\://g)"

		docker --version > /dev/null 2>&1
		if [ $? -ne 0 ]
		then
		        echo "Installing docker..."
		        install_docker
		        DOCKER_WAS_INSTALLED=1

		else
			echo "Docker is already installed."
		fi

		echo "pulling docker images"
		docker pull plviqot/iqot_gw_core_app_x64
		checking_error $?
		docker pull plviqot/iqot_emulator
		checking_error $?

		show_ids 

		echo "enter Gateway and Endpoint IDs to IQoT web app "
		echo "https://iqot.io/devices/"
		echo -e "when You enter IDs to IQoT web app, you can start emulator sample\n"

		if [ $DOCKER_WAS_INSTALLED -eq 1 ]
		then
			echo "Please, run command 'sudo usermod -aG docker \$USER', log out and log in from your host"
		fi

		;;
	start)

		if [ "$2" = "-i" ] || [ "$2" = "--network-interface" ]
		then
			INTERFACE="$3"
		fi

		ifconfig $INTERFACE > /dev/null 2>&1
		if [ $? -ne 0 ]
		then
			echo "invalid network Interface. Please, run script 'emulator.sh start' with '-i' or '--network-interface' option"
			exit 1
		fi

		GATEWAY_ID="gw$(cat /sys/class/net/$INTERFACE/address | sed s/\://g)"
		ENDPOINT_ID="ep$(cat /sys/class/net/$INTERFACE/address | sed s/\://g)"

		echo -e "Before running, please add gateway and emulator IDs to IQoT web app\n"
		show_ids
		echo "press 'Enter' to continue after You entered IDs to IQoT web app" 
		read
		echo "preparing coreapp"
		docker run -dt --mac-address $(cat /sys/class/net/$INTERFACE/address) --name $IQOT_COREAPP plviqot/iqot_gw_core_app_x64
		checking_error $?
		echo "preparing emulator"
		sleep 5
		docker run -dt --network container:$IQOT_COREAPP -e END_ID=$ENDPOINT_ID --name $IQOT_EMULATOR plviqot/iqot_emulator
		checking_error $?
		echo "emulator is running.."
		;;
	stop)
		echo "stopping emulator"
		docker rm -f $IQOT_COREAPP $IQOT_EMULATOR
		echo "emulator is stopped"
		;;
	remove)
		echo "removing images"
		docker rm -f $IQOT_COREAPP $IQOT_EMULATOR > /dev/null 2>&1
		docker rmi -f plviqot/iqot_gw_core_app_x64 plviqot/iqot_emulator
		echo "emulator is removed"
		;;
esac
