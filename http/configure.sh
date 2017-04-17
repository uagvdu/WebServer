#########################################################################
# File Name: configure.sh
# Created Time: 2017年04月03日 星期一 16时37分25秒
#########################################################################
#!/bin/bash




#echo helo
##SRC=$(ls ${PATH}/*.c)
#SRC=$(ls ./*.c)
#PATH=$(pwd)
#INCLUDE=$PATH
#OBJ=$(echo ${SRC} |sed 's/\.c/\.o/g')
#LIB=-lpthread
#
#cat<<EOF >Makefile
#${SERVER_NAME}:
#
#EOF
#
#echo $SRC





#SRC
ROOT_PATH=$(pwd)
INCLUDE=$ROOT_PATH
SRC=$(ls *.c|tr '\n' ' ')
OBJ=$(ls *.c |sed 's/\.c/\.o/g'|tr '\n' ' ')
SERVER_NAME=httpd
CC=gcc
LIB=-lpthread

#top Makefile
cat <<EOF >Makefile
.PHONY:all
all:${SERVER_NAME} cgi
#
#
# all依赖几个目标，一个make就会执行目标依赖的方法生成几个目标：不管是不是伪目标
#
#
${SERVER_NAME}:${OBJ}
	${CC} -o \$@ \$^ ${LIB} ${GDB}
%.o:%.c
	${CC} -c \$<

.PHONY:cgi
cgi:
	cd cgi;make;make output; cd -

.PHONY:clean
clean:
	rm -rf *.o $SERVER_NAME output;cd cgi;make clean;cd -;rm cgi_math
.PHONY:output
output:
	mkdir output
	cp -rf wwwroot output/
	mkdir -p output/wwwroot/cgi_bin
	cp -f cgi_math output/wwwroot/cgi_bin/
	cp ${SERVER_NAME} output/
	cp -rf  log output/
	cp -rf conf output/
	cp -r http_ctl.sh output/
EOF


#CGI
CGI_PATH=$ROOT_PATH/cgi/
#MATH_SRC=$(ls $CGI_PATH | grep 'math' |grep -E '.c$') #grep -E '.c'匹配以.c结尾的行
MATH_SRC=$(ls ${CGI_PATH}*.c)
MATH_CGI_BIN=cgi_math

#cgi Makefile
cat <<EOF > $CGI_PATH/Makefile
${MATH_CGI_BIN}:$MATH_SRC
	$CC -o \$@ \$^
.PHONY:clean
clean:
	rm -f $MATH_CGI_BIN
.PHONY:output
output:
	cp $MATH_CGI_BIN ..

EOF







