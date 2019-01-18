all:main_server.o functions_server.o main_crawler.o functions_crawler.o common.o
	gcc -o myhttpd main_server.o functions_server.o common.o -lm -g -pthread
	gcc -o mycrawler main_crawler.o functions_crawler.o common.o -lm -g -pthread

main_server.o:main_server.c
	gcc -c main_server.c -g

functions_server.o:functions_server.c
	gcc -c functions_server.c -g

main_crawler.o:main_crawler.c
	gcc -c main_crawler.c -g

functions_crawler.o:functions_crawler.c
	gcc -c functions_crawler.c -g

common.o:common.c
	gcc -c common.c -g

clean:
	rm -f ./myhttpd ./main_server.o ./functions_server.o ./mycrawler ./main_crawler.o ./functions_crawler.o common.o
