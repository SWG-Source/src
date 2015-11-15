#!/bin/sh
#

STATUS=`./LoginPing_d -- @loginPing.cfg`
if [ "$STATUS" != "Ok" ]; then
	./LoginServer_d -- @loginServer.cfg &
	echo "LoginServer not responding"
fi
