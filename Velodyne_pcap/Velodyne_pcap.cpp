/*
* Copyright (c) 1999 - 2005 NetGroup, Politecnico di Torino (Italy)
* Copyright (c) 2005 - 2006 CACE Technologies, Davis (California)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* 3. Neither the name of the Politecnico di Torino, CACE Technologies
* nor the names of its contributors may be used to endorse or promote
* products derived from this software without specific prior written
* permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifdef _MSC_VER
/*
 * we do not want the warnings about the old deprecated and unsecure CRT functions
 * since these examples can be compiled under *nix as well
 */
//#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include <pcap.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#define	PCAP_MAGIC			0xa1b2c3d4

/* 4 bytes IP address */
typedef struct {
	u_char byte1;
	u_char byte2;
	u_char byte3;
	u_char byte4;
}ip_address_t;

/* IPv4 header */
typedef struct {
	u_char	ver_ihl;		// Version (4 bits) + Internet header length (4 bits)
	u_char	tos;			// Type of service 
	u_short tlen;			// Total length 
	u_short identification; // Identification
	u_short flags_fo;		// Flags (3 bits) + Fragment offset (13 bits)
	u_char	ttl;			// Time to live
	u_char	proto;			// Protocol
	u_short crc;			// Header checksum
	ip_address_t saddr;		// Source address
	ip_address_t daddr;		// Destination address
	u_int	op_pad;			// Option + Padding
} ip_header_t;

/* UDP header */
typedef struct{
	u_short sport;			// Source port
	u_short dport;			// Destination port
	u_short len;			// Datagram length
	u_short crc;			// Checksum
} udp_header_t;

int gettimeofday(struct timeval* tv){
	union {
		long long ns100;
		FILETIME ft;
	} now;

	GetSystemTimeAsFileTime(&now.ft);
	tv->tv_usec = (long)((now.ns100 / 10LL) % 1000000LL);
	tv->tv_sec = (long)((now.ns100 - 116444736000000000LL) / 10000000LL);
	return (0);
}

int write_file_header(FILE* output_file){
	struct pcap_file_header fh;

	fh.magic = PCAP_MAGIC;

	fh.version_major = 2;
	fh.version_minor = 4;

	fh.thiszone = 0;

	fh.sigfigs = 0;
	fh.snaplen = 102400;

	fh.linktype = DLT_EN10MB;

	fwrite(&fh, sizeof(fh), 1, output_file);
	return 0;
}

// 构造以时间作为前缀的.pcap文件名
int getFileName(char* filename, int len, const char* veledyne_device_type){
	struct timeval tp;
	gettimeofday(&tp);
	struct tm *ltime;
	char timestr[32];

	time_t local_tv_sec;

	// convert the timestamp to readable format
	local_tv_sec = tp.tv_sec;
	ltime = localtime(&local_tv_sec);

	strftime(timestr, sizeof(timestr), "%x %X", ltime);
	printf("--------- Time: %s [usec]%d ---------\n", timestr, tp.tv_usec);
	strftime(timestr, sizeof(timestr), "%Y%m%d%H%M%S", ltime);
	_snprintf(filename, len, ".\\pcap\\%s_%s.pcap", veledyne_device_type, timestr);
	return 0;
}

/* prototype of the packet handler */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);

char* netmask_len2str(int mask_len, char* mask_str){
	int i;
	int i_mask;

	for (i = 1, i_mask = 1; i < mask_len; i++){
		i_mask = (i_mask << 1) | 1;
	}

	i_mask = htonl(i_mask << (32 - mask_len));
	strcpy(mask_str, inet_ntoa(*((struct in_addr *)&i_mask)));

	return mask_str;
}

int netmask_str2len(char* mask){
	int netmask = 0;
	unsigned int mask_tmp;

	mask_tmp = ntohl((int)inet_addr(mask));
	while (mask_tmp & 0x80000000){
		netmask++;
		mask_tmp = (mask_tmp << 1);
	}

	return netmask;
}

int main(int argc, char** argv){
	char velodyne_device_type[16] = "Vel_32";
	if (argc > 1){
		strcpy(velodyne_device_type, argv[1]);
	}

	pcap_if_t *alldevs;
	pcap_if_t *d;
	int inum;
	int i = 0;
	pcap_t *adhandle;
	char errbuf[PCAP_ERRBUF_SIZE];
	char packet_filter[] = "src 192.168.1.201 and port 2368";

	/* Retrieve the device list */
	if (pcap_findalldevs(&alldevs, errbuf) == -1){
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		exit(1);
	}

	/* Print the list */
	for (d = alldevs; d; d = d->next){
		printf("%d. %s", ++i, d->name);
		if (d->description)
			printf(" (%s)\n", d->description);
		else
			printf(" (No description available)\n");
	}

	if (i == 0){
		printf("\nNo interfaces found! Make sure WinPcap is installed and WinPcap Driver is running.\n");
		return -1;
	}

	printf("Enter the interface number (1-%d):", i);
	scanf("%d", &inum);

	/* Check if the user specified a valid adapter */
	if (inum < 1 || inum > i){
		printf("\nAdapter number out of range.\n");

		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* Jump to the selected adapter */
	for (d = alldevs, i = 0; i< inum - 1; d = d->next, i++);

	/* Open the adapter */
	if ((adhandle = pcap_open_live(d->name,		// name of the device
		65536,									// portion of the packet to capture. 
												// 65536 grants that the whole packet will be captured on all the MACs.
		PCAP_OPENFLAG_PROMISCUOUS,				// promiscuous mode (nonzero means promiscuous)
		1000,									// read timeout
		errbuf									// error buffer
		)) == NULL){
		fprintf(stderr, "\nUnable to open the adapter. %s is not supported by WinPcap\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	/* check the link layer. we support only ethernet for simplicity. */
	if (pcap_datalink(adhandle) != DLT_EN10MB){
		fprintf(stderr, "\nthis program works only on ethernet networks.\n");
		/* free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}


	bpf_u_int32 netmask;
	bpf_u_int32 net;
	bpf_program filter;

	pcap_lookupnet(d->name, &net, &netmask, errbuf);

	if (pcap_compile(adhandle, &filter, packet_filter, 1, 0xffffff) <0){
		fprintf(stderr, "\nUnable to compile the packet filter. Check the syntax.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	// set the filter
	if (pcap_setfilter(adhandle, &filter)<0){
		fprintf(stderr, "\nError setting the filter.\n");
		/* Free the device list */
		pcap_freealldevs(alldevs);
		return -1;
	}

	FILE* output;
	char filename[64];
	getFileName(filename, sizeof(filename), velodyne_device_type);
	output = fopen(filename, "wb");
	if (output == NULL){
		printf("Fail to create pcap file......\n");
		return 0;
	}

	write_file_header(output);

	printf("\nlistening on %s... Press Ctrl+C to stop...\n", d->description);

	/* At this point, we don't need any more the device list. Free it */
	pcap_freealldevs(alldevs);

	/* start the capture */
	pcap_loop(adhandle, 0, packet_handler, reinterpret_cast<u_char*>(output));

	fclose(output);

	return 0;
}

/* Callback function invoked by libpcap for every incoming packet */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data){
	struct tm* ltime;
	char timestr[16];
	ip_header_t* ih;
	udp_header_t* uh;
	u_int ip_len;
	u_short sport, dport;
	time_t local_tv_sec;

	// unused parameter
	FILE* output = reinterpret_cast<FILE*>(param);
	fwrite(header, sizeof(pcap_pkthdr), 1, output);
	fwrite(pkt_data, header->caplen, 1, output);

	/* convert the timestamp to readable format */
	local_tv_sec = header->ts.tv_sec;
	ltime = localtime(&local_tv_sec);
	strftime(timestr, sizeof timestr, "%H:%M:%S", ltime);

	/* print timestamp and length of the packet */
	//printf("%s.%.6d len:%d headlen:%d ", timestr, header->ts.tv_usec, header->len, sizeof(pcap_pkthdr));

	/* retireve the position of the ip header */
	ih = (ip_header_t *)(pkt_data + 14);									//length of ethernet header

	/* retireve the position of the udp header */
	ip_len = (ih->ver_ihl & 0xf) * 4;
	uh = (udp_header_t *)((u_char*)ih + ip_len);

	/* convert from network byte order to host byte order */
	sport = ntohs(uh->sport);
	dport = ntohs(uh->dport);

	/* print ip addresses and udp ports */
	//printf(" IP: %d.%d.%d.%d:%d -> %d.%d.%d.%d:%d\n",
	//	ih->saddr.byte1,
	//	ih->saddr.byte2,
	//	ih->saddr.byte3,
	//	ih->saddr.byte4,

	//	sport,

	//	ih->daddr.byte1,
	//	ih->daddr.byte2,
	//	ih->daddr.byte3,
	//	ih->daddr.byte4,
	//	
	//	dport);
}
