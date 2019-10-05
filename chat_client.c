#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "socket.h"


#ifndef PORT
  #define PORT 50001
#endif
#define BUF_SIZE 128
#define IP "127.0.0.1"

int main(void) {
    char username[BUF_SIZE+1];
    printf("Enter Username: ");
    scanf("%s", username);

    char KEY[BUF_SIZE+1];
    printf("Enter Key: ");
    scanf("%s", KEY);
    printf("\n");

    // Initialize connection
    int sock_fd = connect_to_server(PORT, IP);

    // Read input from the user, send it to the server, and then accept the
    // echo that returns. Exit when stdin is closed.
    char buf[BUF_SIZE + 1];
    char msg[BUF_SIZE + 1];
	char enc[257];
	char dec[257];

    fd_set fdset, fdall;

    FD_ZERO(&fdall);

    FD_SET(sock_fd, &fdall);
    FD_SET(fileno(stdin), &fdall);

    while (1) {

        fdset = fdall;

        if (select(sock_fd + 1, &fdset, NULL, NULL, NULL) < 0) {
            perror("select");
            return 1;
        }

        // If  there is data  from  stdin , send to server
        if (FD_ISSET(fileno(stdin), &fdset)) {
            int num_read = read(fileno(stdin), buf, BUF_SIZE);
            if (num_read == 0) {
                break;
            }
            buf[num_read] = '\0';

            // This fixes the fact that stdout goes back into stdin
            const char delim[2] = "\n";
            char *rest;
           	char *text = strtok_r(buf, delim, &rest);
            if ((rest != NULL) && (rest[0] != '\0')) {
            	text = rest;
            }

            snprintf(msg, BUF_SIZE, "[%s]: %s\n", username, text); 

            // encrypt message before sending

    		snprintf(enc, 100, "echo '%s' | openssl enc -a -e -aes-256-cbc -k %s", msg, KEY);

    		FILE *fp = popen(enc, "r");
			while (fgets(msg, BUF_SIZE, fp) != NULL) {
				continue;				
			}
			pclose(fp);

			// write encoded message to server

            int num_written = write(sock_fd, msg, strlen(msg));
            if (num_written != strlen(msg)) {
                perror("client: write");
                close(sock_fd);
                exit(1);
            }
        }

        // Read from server
        if (FD_ISSET(sock_fd, &fdset)) {
            int num_read = read(sock_fd, buf, BUF_SIZE);
            buf[num_read] = '\0';

            if (strncmp(buf, "[SERVER]", 8) == 0) { // if server command, dont decrypt since its plaintext
            	printf("%s", buf);
	            if (strncmp(buf, "[SERVER] Shutdown.", 18) == 0) {
	            	exit(0);
	            }
        	} else { // if non-server command decrypt it
        		snprintf(dec, 100, "echo '%s' | openssl enc -a -d -aes-256-cbc -k %s", buf, KEY);
        		FILE *fp = popen(dec, "r");
				while (fgets(msg, BUF_SIZE, fp) != NULL) {
					printf("\033[1m%s\033[0m", msg);
				}
				pclose(fp);
	        }
        }
    }

    close(sock_fd);
    return 0;
}
