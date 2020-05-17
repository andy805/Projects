#ifndef GETHOSTBYNAME6_H
#define GETHOSTBYNAME6_H


uint8_t * gethostbyname6(const char * hostName);
char * getIPAddressString(uint8_t * ipAddress);
uint8_t * getIPAddress6(const char * hostName, struct sockaddr_in6 * aSockaddr);


#endif
