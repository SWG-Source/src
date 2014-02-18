# SWGNGE

## Ubuntu Environment Setup

Note that the server only supports 32bit linux platforms at this time.

	sudo apt-get install build-essential zlib1g-dev libpcre3-dev cmake libboost-dev libxml2-dev libncurses5-dev flex bison git-core alien libaio1

### Oracle Instant Client

Follow this guide to get the oracle client set up. 

https://help.ubuntu.com/community/Oracle%20Instant%20Client

### Oracle Java 7

	sudo add-apt-repository ppa:webupd8team/java
	sudo apt-get update
	sudo apt-get install oracle-java7-installer
	sudo apt-get install oracle-java7-set-default

You must log out and back in (or just reboot) for the $JAVA_HOME and $ORACLE_HOME to be set properly after these tools are installed. This is required in order for cmake to be able to find these tools when configuring the source code for building.

## Build code

	git clone git@bitbucket.org:apathyboy/swg-src.git
	mkdir swg-src/build
	cd swg-src/build
	cmake ..
	make


