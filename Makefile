all: send_message receive_and_send_message send_and_receive_message send_insta_message send_limited_repeated_messages send_unlimited_repeated_messages

send_message: send_message.c
	gcc -Wall -g -o $@ send_message.c

receive_and_send_message: receive_and_send_message.c
	gcc -Wall -g -o $@ receive_and_send_message.c -lpthread

send_and_receive_message: send_and_receive_message.c
	gcc -Wall -g -o $@ send_and_receive_message.c -lpthread

send_insta_message: send_insta_message.c
	gcc -Wall -g -o $@ send_insta_message.c

send_limited_repeated_messages: send_limited_repeated_messages.c
	gcc -Wall -g -o $@ send_limited_repeated_messages.c

send_unlimited_repeated_messages: send_unlimited_repeated_messages.c
	gcc -Wall -g -o $@ send_unlimited_repeated_messages.c

clean:
	rm -rv send_message receive_and_send_message send_and_receive_message send_insta_message send_limited_repeated_messages send_unlimited_repeated_messages
