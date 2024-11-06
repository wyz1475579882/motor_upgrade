#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "can-fd-host.h"

int main(int argc, char ** argv)
{
	if(canFd_openDriver())
	{
		if(argv[1] != NULL)
		{
			if(strcmp(argv[1],"-Version")==0)
			{
				canFd_getVersion();
			}
			if(strcmp(argv[1],"-LimitOff")==0)
			{
				canFd_closeLimit();
			}
			if(strcmp(argv[1],"-LimitOn")==0)
			{
				canFd_openLimit(1);
			}
			if(strcmp(argv[1],"-MotorUpgrade")==0)
			{
				if(argv[2] != NULL && argv[3] != NULL)
				{
					canFd_openLimit(0);
					canFd_motorUpgradeSendInfo();
					canFd_motorUpgradeSendData(0);
					canFd_motorUpgradeSendData(1);
					canFd_motorUpgrade(argv[2][0] & 0x0f,argv[3][0] & 0x0f,argv[4][0] & 0x0f);
				}
				else
				{
					printf("please enter parameter:-Version ,-LimitOff,-LimitOn,-MotorUpgrade $type $id\n\n");
					return 0;
				}
			}
		}
		else
		{
			printf("please enter parameter:-Version ,-LimitOff,-LimitOn,-MotorUpgrade $type $id\n\n");
			return 0;
		}
	
		canFd_loopStart();
	}
	return 0;
}