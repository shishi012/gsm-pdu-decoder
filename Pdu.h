

//###########################################################################
// @INCLUDE
//###########################################################################
#include "MatrixTypes.h"

//###########################################################################
// @DEFINES
//###########################################################################
#define SMS_PDU_MAX_LEN					175
#define SMS_PDU_USER_DATA_MAX_LEN		140  /* 140 octets */
#define TRUNCATED_PDU_DATA_LEN			134
#define SMS_GSM7BIT_MAX_LEN				160
#define TRUNCATED_GSM_DATA_LEN			153
#define ADDR_OCTET_MAX_LEN				20	 /* 10 Octets */
#define TIME_STAMP_OCTET_MAX_LEN		14	 /* 7 Octets */
#define UTF8_CHAR_LEN					4	 /* 4 bytes */
#define GSM_7BIT						0x00
#define ANSI_8BIT						0x01  // TODO
#define UCS2_16BIT						0x02  // TODO
#define USER_DATA_HEADER_INDICATION		0x40
#define STATUS_REPORT_INDICATOR			0x20
#define MSG_REF_NO_DEFAULT				0x00
#define UDH_CONCATENATED_MSG_LEN		0x05
#define IE_CONCATENATED_MSG_LEN			0x03
#define MORE_MSG_TO_SEND				0x04
#define TRUE							 1
#define FALSE							 0
#define LONG_SMS_TEXT_MAX_LEN			700
//###########################################################################
// @ENUMERATOR
//###########################################################################
/* Error Values */
enum
{
	ERR_MSG_TYPE = 0,
	ERR_CHAR_SET,
	ERR_PHONE_TYPE_OF_ADDR,
	ERR_PHONE_NUM_PLAN,
	ERR_PROTOCOL_ID,
	ERR_DATA_CODE_SCHEME
};

/* Message Type indication */
enum	
{
	MSG_TYPE_SMS_DELIVER = 0x00,
	MSG_TYPE_SMS_SUBMIT = 0x01,
	MSG_TYPE_SMS_STATUS_REPORT = 0x02
};

/* Type of Number */
enum
{
   NUM_TYPE_UNKNOWN = 0x00,
   NUM_TYPE_INTERNATIONAL = 0x01,
   NUM_TYPE_NATIONAL = 0x02,
   NUM_TYPE_ALPHANUMERIC = 0x05
};

/* Numbering Plan Indicator */
enum
{
   NUM_PLAN_UNKNOWN = 0x00,
   NUM_PLAN_ISDN = 0x01
};

/* Validity Period type */
enum
{
   VLDTY_PERIOD_DEFAULT = 0x00,
   VLDTY_PERIOD_RELATIVE = 0x10,
   VLDTY_PERIOD_ABSOLUTE = 0x18
};

/* User Data Header Information Element identifier */
enum
{
   IE_CONCATENATED_MSG = 0x00,
   IE_PORT_ADDR_8BIT = 0x04,
   IE_PORT_ADDR_16BIT = 0x05
};

/* Message State */
enum
{
	MSG_DELIVERY_FAIL = 0,
	MSG_DELIVERY_SUCCESS
};

//###########################################################################
// @DATATYPE
//###########################################################################
/* PDU Decode Descriptor */
typedef struct
{
	UINT8 smscAddrLen;						/* Length of Service Center Number  */
	UINT8 smscNpi;							/* Numbering Plan Indicactor */
	UINT8 smscTypeOfAddr;					/* Type of Address of Service Center Number */
	char smscAddr[ADDR_OCTET_MAX_LEN + 1];	/* Service Center Number */
	UINT8 firstOct;							/* First octet of PDU SMS */							
	BOOL isHeaderPrsnt;						/* User data header indicator */
	UINT8 msgRefNo;							/* Message Reference Number */
	
	UINT8 phoneAddrLen;						/* Lenght of Phone Number */
					
	UINT8 phoneTypeOfAddr;					/* Type of Address of Phone Number */
	
	char phoneAddr[ADDR_OCTET_MAX_LEN + 1];	/* Phone Number */
	
	UINT8 protocolId;						/* Protocol Identifier */
	UINT8 dataCodeScheme; 					/* Data Coding scheme */
	UINT8 msgType;						    /* Message Type */ 
	BOOL isWapPushMsg;						/* WAP-PUSH SMS */
	BOOL isFlashMsg;						/* FLASH SMS */
	BOOL isStsReportReq;					/* Staus Report Flag */
	BOOL isMsgWait;							/* Message Waiting */
	
	UINT8 usrDataFormat;					/* User Data Coding Format */
	
	char timeStamp[TIME_STAMP_OCTET_MAX_LEN + 1];		 /* Service Center Time Stamp */
	char dischrgTimeStamp[TIME_STAMP_OCTET_MAX_LEN + 1]; /* Discharge Time Stamp */
	
	UINT8 vldtPrd;							 /* Validity Period */
	UINT8 vldtPrdFrmt;						 /* Validity Period Format */
	
	UINT8 usrDataLen;							    				/* User Data Length */
	UINT8 usrData[SMS_GSM7BIT_MAX_LEN * UTF8_CHAR_LEN + 1];   		/* User Data for GSM_7bit, ANSI_8bit & UCS2_16bit*/
	UINT8 udhLen;									/* User Data Header Length */
	UINT8 udhInfoType;								/* Type of User Data Header */
	UINT8 udhInfoLen;								/* User Data Header information length */
	UINT8 concateMsgRefNo; 							/* Concatenated Message Reference Number */
	UINT8 concateTotalParts;						/* Maximum Number of concatenated messages */
	UINT8 concateCurntPart;							/* Sequence Number of concatenated messages */
	BOOL isConcatenatedMsg;							/* Concatenated Msg or Not */
	UINT8 smsSts;					  				/* Status of SMS */
	UINT16 srcPortAddr;								/* Source Port Address */
	UINT16 destPortAddr;							/* Destination Port Address */
	BOOL isDeliveryReq;
	DATE_DESC date;
	TIME_DESC time;
	
} PDU_DESC;

typedef struct
{
	UINT8 day;
	UINT8 month;
	UINT8 year;	
	
} DATE_DESC;

typedef struct
{
	UINT8 hour;
	UINT8 minute;
	UINT8 second;	
	
} TIME_DESC;

//###########################################################################
// @PROTOTYPE
//###########################################################################
BOOL DecodePduData(char *pGsmPduStr, PDU_DESC *pPduDecodeDesc, UINT8 *pError);
BOOL EncodePduData(PDU_DESC *pPduEncodeDesc, UINT8 *pGsmPduStr, UINT16 gsmPduStrLen);

#endif	// PDU_H

