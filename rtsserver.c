#include "alex.h"
#include "alexether.h"

#define PORT 9977

int sock, port, mode, numofplayers, keyboard_fd;
char msg[1024];
FILE *fp;

struct player {
	struct player *next;
	struct sockaddr_in ca;
	double playernum, joined, lastpong;
	char player_ip[20];
};

struct player *first_player, *last_player, *sp;

struct player*
idplayer (struct sockaddr_in *ca)
{
	struct player *pp;

	for (pp = first_player; pp; pp = pp->next) {
		if (pp->ca.sin_addr.s_addr == ca->sin_addr.s_addr) {
			return (pp);
		}
	}

	return (NULL);
}

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
sendtoall (char str[1024])
{
	struct player *pp;

	printf (str);
	for (pp = first_player; pp; pp = pp->next) {
		if (pp->joined) {
			sendpacket (str, &(pp->ca));
		}
	}
}

void
createplayer (struct sockaddr_in *ca)
{
	double pn, now;
	struct player *pp, *pl;

	pn = 0;
	now = get_secs ();
	msg[0] = 0;

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
					pl->lastpong = now;
					return;
				} else {
					sprintf (msg, "player %g joined\n",
						pl->playernum);
					pl->lastpong = now;
					numofplayers++;
					return;
				}
			}
		}
		pp->playernum = last_player->playernum + 1;
		last_player->next = pp;
	}
	
	last_player = pp;

	pp->ca = *ca;
	pp->lastpong = now;

	sprintf (msg, "player %g joined\n", pp->playernum);
}

int
main (int argc, char **argv)
{
	struct sockaddr_in sa, ca;
	double now;
	char c, line[1024], msg[1024], buff[1024];
	int n, slen;
	socklen_t ca_len;
	
	mode = 0;
	numofplayers = 0;
	now = get_secs ();

	if (argc != 2) {
		printf ("usage: rtsserver mapname.rtsmap\n");
		return (1);
	}
	
	fp = fopen (argv[1], "r");
		
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
	
	ca_len = sizeof (ca);

	while (1) {
		usleep (1e4);
		if ((n = recvfrom (sock, buff, sizeof buff - 1, 0,
				   (struct sockaddr*) &ca, &ca_len)) > 0) {

			slen = strlen (buff) - 1;
			while (slen > 0 && (isspace (buff[slen])) != 0) {
				buff[slen] = 0;
				slen--;
			}

			if (numofplayers == 0) {
				createplayer (&ca);
				numofplayers++;
			}

			if ((sp = idplayer (&ca)) == NULL) {
				createplayer (&ca);
				if ((sp = idplayer (&ca))
				    == NULL) {
					printf ("unable to id player, "
						"and/or failed to create\n");
				}
				numofplayers++;
			}

			printf ("received packet: '%s' from player %g\n",
				buff, sp->playernum);

			switch (mode) {
			case 0:
				if (strcmp (buff, "join request") == 0) {
					if (sp->joined == 0) {
						sp->joined = 1;
						sprintf (msg, "player %g "
							 "joined\n",
							 sp->playernum);
						sendtoall (msg);
					} else {
						printf ("player %g already "
							"joined\n",
							sp->playernum);
						sprintf (msg, "you have "
							 "already joined\n");
						sendpacket (msg, &(sp->ca));
					}
				} else if (strcasecmp (buff, "quit") == 0
					   || strcasecmp (buff, "q") == 0) {
					if (sp->joined == 1) {
						sp->joined = 0;
						sprintf (msg, "player %g "
							 "left\n",
							 sp->playernum);
						sendtoall (msg);
					} else {
						printf ("player %g already "
							"left\n",
							sp->playernum);
						sprintf (msg, "you have "
							 "already left\n");
						sendpacket (msg, &(sp->ca));
					}
				} else if (strcmp (buff, "start game")
					   == 0) {
					mode = 1;
					sendtoall ("play mode activated, "
						   "joining no longer "
						   "allowed\n");
				} else {
					printf ("invalid packet received\n");
				}
				break;
			case 1:
				if (strcmp (buff, "base map request")
				    == 0 ) {
					c = getc (fp);
					msg[0] = 0;

					while (c != EOF) {
						ungetc (c, fp);
						fgets (line, sizeof line, fp);
						
						if (msg[0] == 0) {
							sprintf (msg, "%s",
								 line);
						} else {
							sprintf (msg, "%s%s",
								 msg, line);
						}
						c = getc (fp);
					}
					sendpacket (msg,
						    &(sp->ca));
					rewind (fp);
				} else {
					printf ("invalid packet received\n");
				}
				break;
			}
		}
	}
	return (0);
}
