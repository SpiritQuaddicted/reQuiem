/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// net_dgrm.h


#ifndef _WIN32
	/*
	 * Internet address (from winsock.h)
	 */
	struct in_addr {
			union {
					struct { unsigned char/*u_char*/ s_b1,s_b2,s_b3,s_b4; } S_un_b;
					struct { unsigned short/*u_short*/ s_w1,s_w2; } S_un_w;
					unsigned long /*u_long*/ S_addr;
			} S_un;
	#define s_addr  S_un.S_addr
									/* can be used for most tcp & ip code */
	#define s_host  S_un.S_un_b.s_b2
									/* host on imp */
	#define s_net   S_un.S_un_b.s_b1
									/* network */
	#define s_imp   S_un.S_un_w.s_w2
									/* imp */
	#define s_impno S_un.S_un_b.s_b4
									/* imp # */
	#define s_lh    S_un.S_un_b.s_b3
									/* logical host */
	};
#endif

int Datagram_Init (void);
void Datagram_Listen (qboolean state);
void Datagram_SearchForHosts (qboolean xmit);
qsocket_t *Datagram_Connect (const char *host);
qsocket_t *Datagram_CheckNewConnections (void);
int Datagram_GetMessage (qsocket_t *sock);
int Datagram_SendMessage (qsocket_t *sock, sizebuf_t *data);
int Datagram_SendUnreliableMessage (qsocket_t *sock, sizebuf_t *data);
qboolean Datagram_CanSendMessage (qsocket_t *sock);
qboolean Datagram_CanSendUnreliableMessage (qsocket_t *sock);
void Datagram_Close (qsocket_t *sock);
void Datagram_Shutdown (void);

void PrintStats (const qsocket_t *s);
