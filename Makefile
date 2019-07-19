all: tcp_send_message tcp_receive_and_send_message tcp_send_and_receive_message tcp_send_insta_message tcp_send_limited_repeated_messages tcp_send_unlimited_repeated_messages udp_receive_message udp_send_message udp_send_insta_message

tcp_send_message: tcp_send_message.c
	gcc -Wall -g -o $@ tcp_send_message.c

tcp_receive_and_send_message: tcp_receive_and_send_message.c
	gcc -Wall -g -o $@ tcp_receive_and_send_message.c -lpthread

tcp_send_and_receive_message: tcp_send_and_receive_message.c
	gcc -Wall -g -o $@ tcp_send_and_receive_message.c -lpthread

tcp_send_insta_message: tcp_send_insta_message.c
	gcc -Wall -g -o $@ tcp_send_insta_message.c

tcp_send_limited_repeated_messages: tcp_send_limited_repeated_messages.c
	gcc -Wall -g -o $@ tcp_send_limited_repeated_messages.c

tcp_send_unlimited_repeated_messages: tcp_send_unlimited_repeated_messages.c
	gcc -Wall -g -o $@ tcp_send_unlimited_repeated_messages.c

udp_receive_message: udp_receive_message.c
	gcc -Wall -g -o $@ udp_receive_message.c

udp_send_message: udp_send_message.c
	gcc -Wall -g -o $@ udp_send_message.c

udp_send_insta_message: udp_send_insta_message.c
	gcc -Wall -g -o $@ udp_send_insta_message.c

clean:
	rm -rv tcp_send_message tcp_receive_and_send_message tcp_send_and_receive_message tcp_send_insta_message tcp_send_limited_repeated_messages tcp_send_unlimited_repeated_messages udp_receive_message udp_send_message udp_send_insta_message
