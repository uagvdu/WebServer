.PHONY:all
all:httpd cgi
#
#
# all依赖几个目标，一个make就会执行目标依赖的方法生成几个目标：不管是不是伪目标
#
#
httpd:httpd.o main.o 
	gcc -o $@ $^ -lpthread 
%.o:%.c
	gcc -c $<

.PHONY:cgi
cgi:
	cd cgi;make; cd -

.PHONY:clean
clean:
	rm -rf *.o httpd output;cd cgi;make clean;cd -;rm cgi_math
.PHONY:output
output:
	mkdir output
	cp -rf wwwroot output/
	mkdir -p output/wwwroot/cgi_bin
	cd cgi;make output;cd -
	cp -f cgi_math output/wwwroot/cgi_bin/
	cp httpd output/
	cp -rf  log output/
	cp -rf conf output/
	cp -r http_ctl.sh output/
