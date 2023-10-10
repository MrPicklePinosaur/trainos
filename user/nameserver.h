#ifndef __USER_NAMESERVER_H__
#define __USER_NAMESERVER_H__

void nameserverTask();
void initNameserverTask();

int RegisterAs(const char *name);
int WhoIs(const char *name);

#endif // __USER_NAMESERVER_H__
