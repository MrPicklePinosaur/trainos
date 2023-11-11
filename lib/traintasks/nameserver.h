#ifndef __TRAINTASKS_NAMESERVER_H__
#define __TRAINTASKS_NAMESERVER_H__

void nameserverTask();
void initNameserverTask();

int RegisterAs(const char *name);
int WhoIs(const char *name);

#endif // __TRAINTASKS_NAMESERVER_H__
