#########################################################################
# File Name: http_ctl.sh
# Created Time: Mon 10 Apr 2017 11:17:24 AM CST
#########################################################################
#!/bin/bash


#控制脚本控制程序的执行,位于http目录下，控制程序的执行：
ROOT_PATH=$(pwd)
BIN=$ROOT_PATH/httpd
CONF=${ROOT_PATH}/conf/httpd.conf
PID=$ROOT_PATH/httpd.pid

# 父进程创建子进程执行shell脚本，shell脚本进程创建孙子进程执行脚本命令
# exit 退出当前进程。属于内置命令
#

function Usage()
{
	printf "%s [start(s)]| [stop(t)] | [restart(st)]\n" "$0"
	exit 1 
}


[[ $# -ne 1 ]]&&
{
	Usage
}

function start()
{
	[[ -f $PID ]]&&
	{
		printf "httpd is running \n"
		exit 2
		
	}
	IP=$(grep -E '^IP:'	$CONF|awk -F ':' '{print $2}')
	PORT=$(grep -E '^PORT:' $CONF |awk -F ':' '{print $2}')
	$BIN $IP $PORT
	pidof $(basename $BIN) > $PID 
	printf "start done pid is $(cat $PID)...\n"  

}

function stop()
{
	[[ ! -f $PID ]]&&
	{
		printf "httpd is not running \n"
		exit 3
	}
	
	NUM=$(cat $PID)

	kill -9 $NUM
	rm $PID
	printf "stop done...\n"

}

case  "$1" in
	stop |  t)
		stop
	;;
	start | s)
		start
	;;
	restart | st)
		stop
		start
	;;
	*)
		Usage
		;;
esac



