/* Author: Andrej Barna (xbarna01)
 * Subject: ISA (Network Applications and Network Administration)
 * Year: 2016, winter semester
 */


#include <iostream>
#include <vector>
#include <map>
#include <cstring>
#include <chrono>
#include <ncurses.h>

#include <arpa/inet.h>
#include <pcap/pcap.h>

#include <netinet/ip.h>
#include <netinet/udp.h>

#ifdef __linux__
#include <netinet/ether.h>
#endif


// Special value denoting the beginning of the Options field in the DHCP packet
// Represents IP address 99.130.83.99, in the Network Byte Order
#ifndef DHCP_MAGIC_COOKIE
#define DHCP_MAGIC_COOKIE (0x63538263)
#endif

// Size of the PCAP Error Buffer
#ifndef PCAP_ERRBUF_SIZE
#define PCAP_ERRBUF_SIZE (256)
#endif



// Contains information about the network to be monitored
typedef struct networkInfo {
  in_addr_t address;   // Network Address in Host Byte Order
  in_addr_t masknum;   // Mask in numeric format, for output printing and errors
  in_addr_t mask;      // Mask in the binary format
  uint32_t devcounter; // Counter of devices in the network
} networkInfo_t;

// Holds information necessary for the run of the application
typedef struct netMonData {
  std::vector <networkInfo_t> *networks; // Vector of networks to be monitored
  std::map <in_addr_t, time_t> *devices; // Stores information about devices that
                  // belong to one of monitored networks, consists of the IP
                  // address and time, when the lease of the IP address ends
  uint ethernet_size = 14; // Default Ethernet Header Size
} netMonData_t;

// Structure of the DHCP header, most fields are not used for this application
typedef struct dhcpHeader {
  uint8_t op;
  uint8_t htype;
  uint8_t hlen;
  uint8_t hops;
  uint32_t xid;
  uint16_t secs;
  uint16_t flags;
  in_addr ciaddr;
  in_addr yiaddr;
  in_addr siaddr;
  in_addr giaddr;
  char chaddr[16];
  char sname[64];
  char file[128];
  //char vend[64]; // Used in the BOOTP, in the DHCP it's known as options
} dhcpHeader_t;



// Function prototypes
void printHelp();
int parseArgs(int, char**, std::vector<networkInfo_t>*, std::string &/*, int &*/);
void dhcpPacketHandler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);
networkInfo_t parseAddress(char *);



/* Processes arguments, opens PCAP packet handle, filters packets in the loop,
 * processes their contents and then outputs desired information
 */
int main(int argc, char **argv){

  std::string pcapfile = "";
  //int logint = 0;
  netMonData_t netData;
  netData.networks = new std::vector<networkInfo_t>;
  netData.devices = new std::map<in_addr_t, time_t>;
  int pstate = 0;
  pstate = parseArgs(argc, argv, netData.networks, pcapfile/*, logint*/);
  if(pstate){
    delete netData.networks;
    delete netData.devices;
    if(pstate == EXIT_FAILURE)
      return EXIT_FAILURE;
    return EXIT_SUCCESS;
  }


  char errbuf[PCAP_ERRBUF_SIZE];  // error buffer used by PCAP functions
  pcap_t *handle;                 // packet capture handle
  struct bpf_program filter;      // PCAP filter, used to capture UDP packets on
                                  // ports 67 and 68, used by DHCP messages


  // Either opens interfaces for monitoring, or opens the specified PCAP file
  if (pcapfile == "" && (handle = pcap_open_live("any", BUFSIZ, 1, 1000, errbuf)) == NULL){
    std::cerr << "Couldn't open an interface for monitoring! Application needs to be run with root privileges!\n";
    return EXIT_FAILURE;
  }
  else if(pcapfile != "" && (handle = pcap_open_offline(pcapfile.data(), errbuf)) == NULL){
    std::cerr << "Couldn't open the PCAP file!\n";
    return EXIT_FAILURE;
  }

  // Ethernet Header is 2 bytes bigger when all interfaces are being sniffed on
  if(pcap_datalink(handle) == DLT_LINUX_SLL){
    netData.ethernet_size += 2;
  }

  // Prepares and applies the filter
  if (pcap_compile(handle, &filter, "udp port 67 or udp port 68", 0, PCAP_NETMASK_UNKNOWN) == -1){
    std::cerr << "Couldn't compile a filter for monitoring!\n";
    return EXIT_FAILURE;
  }
  if (pcap_setfilter(handle, &filter) == -1){
    std::cerr << "Couldn't set a filter for monitoring!\n";
    return EXIT_FAILURE;
  }

  // Loop of the processing
  initscr();
  for(;;){
    // Gets packets from the handle and calls the dhcpPacketHandler function to
    // process them
    if (pcap_dispatch(handle, -1, dhcpPacketHandler, (u_char*) &netData) == -1){
      std::cerr << "An error occured during the packet processing!\n";
      return EXIT_FAILURE;
    }

    // Removes devices from the device list, whose lease has expired
    for (auto& devRec: *(netData.devices)) {
      if(devRec.second - time(NULL) <= 0)
        netData.devices->erase(devRec.first);
    }

    // Prints formatted information about networks, using the ncurses library
    mvprintw(0, 0, "IP Prefix");
    mvprintw(0, 23, "Max hosts");
    mvprintw(0, 40, "Allocated addresses");
    mvprintw(0, 65, "Utilization");
    uint row = 1;
    for(std::vector<networkInfo_t>::iterator net = netData.networks->begin(); net != netData.networks->end(); ++net){
      struct in_addr naddr;
      naddr.s_addr = htonl(net->address);
      mvprintw(row, 0,"%s/%d", inet_ntoa(naddr), net->masknum);
      mvprintw(row, 23,"%ld",  ~net->mask + (net->masknum > 30 ? 1 : -1));
      mvprintw(row, 40,"%d", net->devcounter);
      mvprintw(row, 65,"%.2f%%", 100.0 * ((float) net->devcounter)/((float) ~net->mask + (net->masknum > 30 ? 1.0 : -1.0)));
      row++;
      //std::cout << inet_ntoa(naddr) << "/" << net->masknum << "\t\t" << (~net->mask) - 1 << "\t\t" << net->devcounter << "\t\t" << net->devcounter/((~net->mask) - 1) << std::endl;
    }
  	refresh();
  }

  // Closes the handle and frees allocated space
  pcap_close(handle);

  delete netData.networks;
  delete netData.devices;

  return EXIT_SUCCESS;
}


/* Parses arguments and returns the parsed content through references or pointers
 * given by parameters of the function, return code depends on whether the parsing
 * was successful or not
 */
int parseArgs(int argc, char** argv, std::vector<networkInfo_t> *networks, std::string &pcapfile/*, int &logint*/){

  // Prints help
  if(argc == 1 || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")){
    printHelp();
    return 64; // any number that's neither EXIT_SUCCESS, nor EXIT_FAILURE
  }

  // Processes arguments in the loop
  for(int i = 1; i < argc; i++){
    if(!strcmp(argv[i], "-r")){
      if(i+1 == argc){
        std::cerr << "Filename missing after the argument -r!\n";
        printHelp();
        return EXIT_FAILURE;
      }
      pcapfile = argv[i+1];
      i++;
    }
    /*
    else if(!strcmp(argv[i], "-c")){
      if(i+1 == argc){
        std::cerr << "Logging interval missing after the argument -c!\n";
        printHelp();
        return EXIT_FAILURE;
      }
      char *pos;
      logint = strtoul(argv[i+1], &pos, 10);
      if(*pos != strlen(argv[i+1])){
        std::cerr << "Invalid logging interval after the -c argument!\n";
        printHelp();
        return EXIT_FAILURE;
      }
      i++;
    }
    */
    else{
      // Creates a record of the network using parseAddress() and then puts it
      // in the networks vector
      networkInfo_t ni = parseAddress(argv[i]);
      if(ni.masknum == std::numeric_limits<uint32_t>::max()){
        std::cerr << "Network address " << argv[i] << " is in the wrong format!\n";
        printHelp();
        return EXIT_FAILURE;
      }
      networks->push_back(ni);
    }
  }

  // If no networks were stored in the vector (possible if optional arguments were
  // specified), the parsing ends as unsuccessful
  if(!networks->size()){
    std::cerr << "No networks to monitor were specified!\n";
    printHelp();
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}


// Packet handler, which uses the netData structure from the main function, pointer
// to which is the retyped args parameter
void dhcpPacketHandler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet){
  netMonData_t *netData = (netMonData_t *) args;  // netData structure
  struct ip *ipHeader;    // Pointer to the beginning of the IP header
  u_int ipHeaderSize;     // Size of the IP header
  dhcpHeader_t *dheader;  // Pointer to the beginning of the DHCP header
  uint8_t *dopts;         // Pointer to the DHCP options at the end of the DHCP header

  // Sets up pointers
  ipHeader = (struct ip*) (packet + netData->ethernet_size);
  ipHeaderSize = ipHeader->ip_hl * 4;
  dheader = (struct dhcpHeader*) (packet + netData->ethernet_size + ipHeaderSize + sizeof(struct udphdr));
  dopts = (uint8_t *) (packet + netData->ethernet_size + ipHeaderSize + sizeof(struct udphdr) + sizeof(struct dhcpHeader));

  // Checks if the DHCP packet is valid (has magic cookie)
  if(*(uint32_t *)dopts != DHCP_MAGIC_COOKIE)
    return;

  uint32_t ltime = 0;   // Lease time
  uint8_t dmestype = 0; // DHCP Message Type
  for(int i = 4; dopts[i] != 0xff; i++){ // Checks all DHCP options for desired fields
    switch(dopts[i]){
      case 0:  // Padding
        continue;
      case 51: // IP Address Lease Time, for address validity checks
        ltime = ntohl(*((uint32_t *) (dopts + i + 2)));
        break;
      case 53: // DHCP Message Type
        dmestype = (uint8_t) dopts[i + 2];
        break;
      default:;
    }
    i += dopts[i+1] + 1;
  }

  if(dmestype == 7){
    // Message type DHCPRELEASE - if the device with the address from the DHCPRELEASE
    // message was stored in the netData->devices map then its record is removed
    // and counters of networks to which the device belonged are decremented
    in_addr_t devAddr = ntohl(dheader->ciaddr.s_addr);
    netData->devices->erase(devAddr);
    for(std::vector<networkInfo_t>::iterator it = netData->networks->begin(); it != netData->networks->end(); ++it)
      if((devAddr & it->mask) == it->address && it->devcounter)
        it->devcounter--;
  }
  else if(dmestype == 5){
    // Message type DHCPACK - used as confirmation from the server that client's
    // request was granted (the address was assigned or lease was renewed)

    // First checks whether the device from the message belongs to one of
    // monitored networks
    bool save = false;
    in_addr_t devAddr = ntohl(dheader->yiaddr.s_addr);
    for(std::vector<networkInfo_t>::iterator it = netData->networks->begin(); it != netData->networks->end(); ++it)
      if((devAddr & it->mask) == it->address){
        save = true;
        break;
      }

    // If the device belongs to one of monitored networks, then if it's already
    // stored its lease termination time is updated, otherwise this device is
    // added to the map
    if(save){
      std::pair<std::map<in_addr_t, time_t>::iterator, bool> it;
      it = netData->devices->insert(std::pair<in_addr_t, time_t>(devAddr, header->ts.tv_sec + ltime));
      if (!it.second)
        it.first->second = header->ts.tv_sec + ltime;
      else
        for(std::vector<networkInfo_t>::iterator net = netData->networks->begin(); net != netData->networks->end(); ++net)
          if((devAddr & net->mask) == net->address)
            net->devcounter++;
    }
  }
}


// Prints standard help message
void printHelp(){
  std::cout << "Monitors the usage of network addresses in network(s) given by the application's parameters.\n";
  std::cout << " Usage:\n\t./dhcp-stats <ip-prefix> [ <ip-prefix> [ ... ] ]\n";
  std::cout << " where <ip-prefix> is an IP address in CIDR format, eg. 192.168.64.0/24\n";
  std::cout << "For more information check man ./dhcp-stats.1\n";
}


// Parses address of the network in the CIDR format, that was given as program's
// argument. If the address is illegal, then the structure of the network is
// returned with masknum set to -1
networkInfo_t parseAddress(char *arg){
  networkInfo_t g;
  g.address = 0;
  g.masknum = std::numeric_limits<uint32_t>::max();
  g.mask = 0;
  g.devcounter = 0;
  std::string a = arg;
  size_t pos = 0;

  // IP adress parsing
  for(int i = 3; i >= 0; i--){
    if(!a.length() || !isdigit(a[0]))
      return g;
    int x = stoi(a, &pos, 10);
    if(!pos || (i && a[pos] != '.') || (!i && a[pos] != '/') || (x > 255 || x < 0))
      return g;
    g.address *= 256;
    g.address += x;
    a = a.substr(pos + 1);
  }

  // Mask parsing
  if(!a.length() || !isdigit(a[0]))
    return g;
  g.masknum = stoi(a, &pos, 10);
  if(pos != a.length() || g.masknum > 32){
    g.masknum = std::numeric_limits<uint32_t>::max();
    return g;
  }
  if (g.masknum) g.mask -= 1 << 32 - g.masknum;

  // Verification, whether given IP address is an address of the network with
  // the given network mask, ignores networks with prefix 31/32
  if(~g.mask & g.address && g.masknum < 31)
    g.masknum = std::numeric_limits<uint32_t>::max();
  return g;
}
