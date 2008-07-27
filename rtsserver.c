#include "alex.h"
#include "alexether.h"

#define PORT 9977
#define PLAYERMAX 64

FILE *fp;

struct player {
	struct player *next;
	struct sockaddr_in ca;
	int playernum;
	char player_ip[20];
};

struct player *first_player, *last_player;

int
main (int argc, char **argv)
{
	int sock, port, mode, playernum;
	unsigned int ipaddr;
	struct sockaddr_in sa, ca;
	struct player *pp;

	mode = 0;
	playernum = 0;

	if (argc != 2) {
		printf ("usage: rtsserver mapname.rtsmap\n");
		return (1);
	}
	char c, line[1024];
	
	fp = fopen (argv[1], "r");
	
	c = getc (fp);
	
	sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_IP);
	
	if (sock < 0) {
		fprintf (stderr, "unable to create socket\n");
		return (1);
	}
	
	fcntl (sock, F_SETFL, O_NONBLOCK);
	
	memset (&sa, 0, sizeof sa);
	
	port = PORT;
	
	sa.sin_family = AF_INET;
	sa.sin_port = htons (port);
	
	if ((bind (sock, (struct sockaddr*) &sa, sizeof sa)) < 0) {
		fprintf (stderr, "error binding to socket\n");
		return (1);
	}
	
	printf ("listening for udp packets\n");
	
	while (1) {
		char msg[1024], buff[1024];
		int n;
		socklen_t ca_len;
		int ip[4], ipcounter;
		usleep (1e4);
		ca_len = sizeof (ca);
		if (recvfrom (sock, buff, sizeof buff - 1, 0,
			      (struct sockaddr*) &ca, &ca_len) >= 0) {
			
			buff[n] = 0;
			if (buff[n-1] == '\n') {
				buff[n-1] = 0;
			}
			switch (mode) {
			case 0:
				if (strcmp (buff, "join request") == 1) {
					break;
				}
				pp = xcalloc (1, sizeof *pp);
				if (first_player == NULL) {
					first_player = pp;
				} else {
					last_player->next = pp;
				}

				last_player = pp;

				ipaddr = ca.sin_port;
				pp->ca = ca;
				pp->playernum = playernum;

				printf ("%x\n", ipaddr);

				for (ipcounter = 0; ipcounter < 4; ipcounter++) {
					ip[ipcounter] = ipaddr % 256;
					ipaddr >>= 8;
				}
				playernum++;

/*				printf ("%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

				printf ("client joined: \n");*/

				break;
			case 1:
				while (c != EOF && strcmp (buff, "base map request") == 0) {
					ungetc (c, fp);
					fgets (line, sizeof line, fp);
					
					sprintf (msg, "%s", line);
					
					c = getc (fp);
					
					if (sendto (sock, msg, strlen (msg), 0,
						    (struct sockaddr*) &ca,
						    sizeof ca) < 0) {
						fprintf (stderr, "error sending packet\n");
					}
				}
				break;
			}
		}
	}
	return (0);
}
