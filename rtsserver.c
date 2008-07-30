#include "alex.h"
#include "alexether.h"

#define PORT 9977
#define PLAYERMAX 64

int sock, port, mode;
FILE *fp;

struct player {
	struct player *next;
	struct sockaddr_in ca;
	int playernum, joined;
	char player_ip[20];
};

struct player *first_player, *last_player;

void
sendpacket (char str[1024], struct sockaddr_in *ca)
{
	if (sendto (sock, str, strlen (str), 0,
		    (struct sockaddr*) ca,
		    sizeof *ca) < 0) {
		fprintf (stderr, "error sending packet\n");
	}
}

void
sendtoall (char msg[1024])
{
	struct player *pp;

	printf (msg);
	for (pp = first_player; pp; pp = pp->next) {
		sendpacket (msg, &(pp->ca));
	}
}

void
createplayer (struct sockaddr_in *ca)
{
	char msg[1024];
	int pn;
	struct player *pp, *pl;

	pn = 0;

	pp = xcalloc (1, sizeof *pp);
	if (first_player == NULL) {
		first_player = pp;
		pp->playernum = 0;
	} else {
		for (pl = first_player; pl; pl = pl->next) {
			if (pl->ca.sin_addr.s_addr == ca->sin_addr.s_addr) {
				if (pl->joined == 1) {
					sprintf (msg, "ip already in use\n");
					printf ("client failed to join, ip "
						"already in use\n");
					sendpacket (msg, ca);
					return;
				} else {
					pl->joined = 1;
					sprintf (msg, "player %d joined\n",
						pl->playernum);
					sendtoall (msg);
					return;
				}
			}
		}
		pp->playernum = last_player->playernum + 1;
		last_player->next = pp;
	}
	
	last_player = pp;

	pp->joined = 1;
	pp->ca = *ca;

	sprintf (msg, "player %d joined\n", pp->playernum);

	sendtoall (msg);
}

void
rmplayer (struct sockaddr_in *ca)
{
	char msg[1024];
	struct player *pp;

	for (pp = first_player; pp; pp = pp->next) {
		if (pp->ca.sin_addr.s_addr == ca->sin_addr.s_addr) {
			pp->joined = 0;
			sprintf (msg, "player %d has left\n", pp->playernum);
			sendtoall (msg);
			return;
		}
	}
	printf ("failed to remove player\n");
}

int
main (int argc, char **argv)
{
	struct sockaddr_in sa, ca;

	mode = 0;

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
		usleep (1e4);
		ca_len = sizeof (ca);
		if ((n = recvfrom (sock, buff, sizeof buff - 1, 0,
				   (struct sockaddr*) &ca, &ca_len)) >= 0) {
			buff[n] = 0;

			switch (mode) {
			case 0:
				if (strcmp (buff, "join request\n") == 0) {
					createplayer (&ca);
				} else if (strcmp (buff, "leaving\n") == 0) {
					rmplayer (&ca);
				} else if (strcmp (buff, "start game\n")
					   == 0) {
					mode = 1;
					sendtoall ("play mode activated, "
						   "joining no longer "
						   "allowed\n");
				} else {
					printf ("invalid packet recieved\n");
				}
				break;
			case 1:
				if (strcmp (buff, "base map request\n")
				    == 0 ) {
					while (c != EOF) {
						ungetc (c, fp);
						fgets (line, sizeof line, fp);
						
						msg[0] = 0;
						
						if (msg[0] == 0) {
							sprintf (msg, "%s",
								 line);
						} else {
							sprintf (msg,
								 "%s\n%s",
								 msg, line);
						}
						c = getc (fp);
					}
					sendpacket (msg, &ca);
				}
				break;
			}
		}
	}
	return (0);
}
