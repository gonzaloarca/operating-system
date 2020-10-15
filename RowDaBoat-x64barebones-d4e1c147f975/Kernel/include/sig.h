#ifndef SIG_H
#define SIG_H

int sys_createChannel();

int sys_deleteChannel(unsigned int channelId);

int sys_sleep(unsigned int channelId);

int sys_wakeup(unsigned int channelId);

#endif