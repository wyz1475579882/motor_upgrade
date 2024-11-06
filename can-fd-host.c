#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "type.h"
#include "can-fd-host.h"

static int s;
/*The can-fd Tx frame parameter for all canFdIdType*/
static canFdPayload can_fd_payload_id[CAN_FD_ID_INVALID];

/*The can-fd rx callback for total canFdIdType*/
static dataParserCallBack canFd_dataParserCallBack[CAN_FD_ID_INVALID] = {NULL};

extern uint32_t ota_bin_info[];
extern uint8_t ota_bin_data[];
extern uint32_t upgradeCaseGetFileSize(void);
static uint16 packet_id = 2;

uint16 canFd_get2BytesFromStream(const uint8 *src)
{
    uint16 val = 0;

    val = src[1];
    val |= (uint16)src[0] << 8;

    return val;
}

void write_to_socket(int s, struct canfd_frame frame) 
{
    int nbytes = write(s, &frame, sizeof(frame));

    if (nbytes < 0) 
    {
        perror("CAN FD Raw socket write");
    }

    if (nbytes < sizeof(frame)) {
        perror("write: Incomplete CAN FD frame");
    }
}

void canFd_sendData(uint8 *payload,uint16 payload_size,canFdIdType id)
{
	uint16 offset = 0;

	struct canfd_frame tx_frame = 
	{
		.flags   = 0x04,
		.can_id  = id,
		.len     = 0x40,
		.data    = {0}
	};

	tx_frame.data[CAN_FD_PACKET_LENGTH_OFFSET0] = CAN_FD_PACKET_LENGTH_OFFSET0;

	tx_frame.data[CAN_FD_PACKET_LENGTH_OFFSET1] =  payload_size & 0xff;

	while(payload_size - offset > CAN_FD_FRAME_DATA_LENGTH)
	{
		memcpy(&(tx_frame.data[CAN_FD_FRAME_LENGTH_OFFSET]),&payload[offset],CAN_FD_FRAME_DATA_LENGTH);

		offset += CAN_FD_FRAME_DATA_LENGTH;

		write_to_socket(s,tx_frame);
	}

	memset(&(tx_frame.data[CAN_FD_FRAME_LENGTH_OFFSET]),0,CAN_FD_FRAME_DATA_LENGTH);

	memcpy(&(tx_frame.data[CAN_FD_FRAME_LENGTH_OFFSET]),&payload[offset],payload_size - offset);

	offset += CAN_FD_FRAME_DATA_LENGTH;

	write_to_socket(s,tx_frame);
}

void canFd_otaTxSample(uint8 *payload,uint16 payload_size)
{
	canFd_sendData(payload, payload_size, CAN_FD_ID_OTA);
}

static void canFd_payloadParameterInit(void)
{
	for(int j = 0 ; j < CAN_FD_ID_INVALID ; j++)
	{
		memset(&can_fd_payload_id[j],0,sizeof(can_fd_payload_id[j]));
	}
}

bool canFd_dataParserRegister(dataParserCallBack func,canFdIdType id)
{
	if(id >= CAN_FD_ID_INVALID)
	{
		return false;
	}

	canFd_dataParserCallBack[id] = func;
    
	return true;
}

void canFd_imuRxSample(uint8 *payload,uint16 payload_size)
{
	printf("canFd_imuRxSample payload_size %d payload ",payload_size);

	for(int i = 0;i < payload_size;i++)
	{
		printf("%2x ",payload[i]);
	}

	printf("\n");
}

void canFd_otaRxSample(uint8 *payload,uint16 payload_size)
{
    if(payload == NULL)
    {
        printf("canFd_otaRxSample payload NULL\n\n"); 
    }

    switch (payload[1])
    {
        case 26:
            printf("%s\n\n",&payload[2]);
            close(s);
            exit(0);
            break;

        case 25:
            printf("%s\n\n",&payload[2]);
            close(s);
            exit(0);
            break;

        case 24:
            printf("%s\n\n",&payload[2]);
            close(s);
            exit(0);
            break;

        case 27:
            switch (payload[2])
            {
                case 0x00:
                printf("Motor upgrade success\n\n"); 
                break;

                case 0xfe:
                printf("Motor upgrade file verify fail\n\n"); 
                break;

                case 0x02:
                printf("Motor upgrade timeout\n\n"); 
                break;

                default:
                break;
            }
            canFd_closeLimit();
            break;

        case 29:
            canFd_motorUpgradeSendData(packet_id++);
            break;

        default:
            break;
    }
}


int canFd_openDriver(void)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    struct can_filter rfilter[2]; 
    canFd_payloadParameterInit();
    int canfd_enabled = 1;
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_enabled,
                        sizeof(canfd_enabled));

    strcpy(ifr.ifr_name, "can0" );
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(s, (struct sockaddr *)&addr, sizeof(addr));

    rfilter[0].can_id = 0x00;
    rfilter[0].can_mask = CAN_SFF_MASK;

    rfilter[1].can_id = 0x01;
    rfilter[1].can_mask = CAN_SFF_MASK;
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter[0], sizeof(rfilter[0])); 
    setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter[1], sizeof(rfilter[1])); 
    return TRUE;
}
static struct canfd_frame rx_frame;

void canFd_loopStart(void)
{
    int nbytes;
    while(1)
    {
        if(s != 0)
        {
            nbytes = read(s, &rx_frame, sizeof(rx_frame));
        }

        if(nbytes > 2)
        {
            canFd_otaRxSample(&rx_frame.data[2],nbytes-8);
        }
        usleep(100);
    }
    close(s);
}

void canFd_getVersion(void)
{
    uint8 version[62]= {0};
    version[0] = 0xFD;
    version[1] = 26;
    canFd_otaTxSample(version,62);
}

void canFd_openLimit(uint8 recv)
{
    uint8 version[62]= {0};
    version[0] = 0xFD;
    version[1] = 24;
    version[2] = recv;
    canFd_otaTxSample(version,62);
}

void canFd_motorUpgrade(uint8 type,uint8 id,uint8 side)
{
    uint8 data[62]= {0};
    data[0] = 0xFD;
    data[1] = 27;
    data[2] = type;
    data[3] = id;
    data[4] = side;
    canFd_otaTxSample(data,62);
}

void canFd_motorUpgradeSendInfo(void)
{
    uint8 data[62]= {0};
    data[0] = 0xFD;
    data[1] = 28;
    memcpy(&data[2],ota_bin_info,60);
    canFd_otaTxSample(data,62);
}

void canFd_motorUpgradeSendData(uint16 packet_id)
{
    uint8 data[62]= {0};
    printf("total packet:%d finish: %d\n",upgradeCaseGetFileSize()/256,packet_id);
    if(packet_id < (uint16_t)(upgradeCaseGetFileSize()/256))
    {
        for(int8_t index = 0;index < 8;index++)
        {
            data[0] = 0xFD;
            data[1] = 29;
            data[2] = index;
            data[3] = packet_id >> 8 & 0xff;
            data[4] = packet_id & 0xff;
            memcpy(&data[5],&ota_bin_data[index*32+packet_id*256],32);
            canFd_otaTxSample(data,62);
        }
    }
    else
    {
        uint8 last_packet_len = upgradeCaseGetFileSize()-packet_id*256;
        uint8 last_packet[256] = {0};
        memcpy(last_packet,&ota_bin_data[packet_id*256],last_packet_len);

        for(int8_t index = 0;index < 8;index++)
        {
            data[0] = 0xFD;
            data[1] = 29;
            data[2] = index;
            data[3] = packet_id >> 8 & 0xff;
            data[4] = packet_id & 0xff;
            memcpy(&data[5],&last_packet[index*32],32);
            canFd_otaTxSample(data,62);
        }
    }
}

void canFd_closeLimit(void)
{
    uint8 version[62]= {0};
    version[0] = 0xFD;
    version[1] = 25;
    canFd_otaTxSample(version,62);
}