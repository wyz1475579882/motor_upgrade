/*
 * Copyright 2023 Directdrivetech wuyunzhou@directdrivetech.com
 *
 * SPDX-License-Identifier:
 */

#ifndef CAN_FD_HOST_H
#define CAN_FD_HOST_H

#include "type.h"
#include <linux/can.h>
#include <linux/can/raw.h>

/*The can-fd rx callback definition*/
typedef void (*dataParserCallBack)(uint8 *payload, uint16 payload_len);

/* thread stacksize */
#define CAN_FD_THREAD_STACKSIZE (1024)

/* thread priority for read*/
#define CAN_FD_THREAD_READ_PRIORITY (7)

/* thread priority for send*/
#define CAN_FD_THREAD_SEND_PRIORITY (7)

/*can-fd data field lendth*/
#define CAN_FD_FRAME_LENGTH (64)

/*tx total length int data[0] data[1]*/
#define CAN_FD_FRAME_LENGTH_OFFSET (2)

/*Remove the length field, the remaining data length*/
#define CAN_FD_FRAME_DATA_LENGTH (CAN_FD_FRAME_LENGTH - CAN_FD_FRAME_LENGTH_OFFSET)

/*tx total buffer size*/
#define CAN_FD_PACKET_LENGTH (2048)

/*the offset of tx total length high position is data[0]*/
#define CAN_FD_PACKET_LENGTH_OFFSET0 (0)

/*the offset of tx total length low position is data[1]*/
#define CAN_FD_PACKET_LENGTH_OFFSET1 (1)

/**
 * Test bitrates in bits/second.
 */
#define CAN_FD_FRAME_DATA_BITRATE (5000000)


/*The can-fd id from can_frame*/
typedef enum
{
	CAN_FD_ID_IMU         = 0x00, /*Can-fd frame id for imu*/
	CAN_FD_ID_OTA         = 0x01, /*Can-fd frame id for ota*/
	CAN_FD_ID_INVALID     /*Can-fd frame id end*/
}canFdIdType;

/*The can-fd Tx frame parameter*/
typedef struct
{
	uint8  payload[2048];  /*buffer for tx frame*/
	uint16 payload_offset; /*offset record for tx frame*/
	uint16 payload_length; /*length total for tx frame*/
}canFdPayload;

int canFd_openDriver(void);

uint16 canFd_get2BytesFromStream(const uint8 *src);

void write_to_socket(int s, struct canfd_frame frame);

void canFd_sendData(uint8 *payload,uint16 payload_size,canFdIdType id);

void canFd_otaTxSample(uint8 *payload,uint16 payload_size);

void canFd_imuRxSample(uint8 *payload,uint16 payload_size);

void canFd_otaRxSample(uint8 *payload,uint16 payload_size);

bool canFd_dataParserRegister(dataParserCallBack func,canFdIdType id);

void canFd_loopStart(void);

void canFd_getVersion(void);

void canFd_openLimit(uint8 recv);

void canFd_closeLimit(void);

void canFd_motorUpgrade(uint8 type,uint8 id,uint8 side);

void canFd_motorUpgradeSendInfo(void);

void canFd_motorUpgradeSendData(uint16 packet_id);

#endif