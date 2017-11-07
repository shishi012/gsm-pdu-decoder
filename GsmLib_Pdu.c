

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "MatrixTypes.h"
#include "PrintFun.h"
#include "GsmLibType.h"

#include "GsmLib_Pdu.h"
#include "GsmLib.h"

//###########################################################################
// @DEFINES
//###########################################################################
#define UTF7							0x00
#define UTF8							0x04
#define UTF16							0x08
#define ESC_CHR							0x1B
#define	UDH_CONCATENATED_MSG_LEN		0x05
#define	IE_CONCATENATED_MSG				0x00
#define	IE_CONCATENATED_MSG_LEN			0x03

#define ALPHANUMERIC_NUM_PLAN			0x50
//###########################################################################
// @GLOBAL VARIABLE
//###########################################################################

//###########################################################################
// @PROTOTYPES
//###########################################################################
static UINT8 Hex2Ascii(UINT8 hexNibble);
static UINT8 Ascii2Hex(UINT8 asciiChar);
static UINT16 HexBuf2AsciiBuf(UINT8 *hexBuf, UINT8 hexBufLen, char *asciiStrng);
static UINT8 AsciiBuf2HexBuf(char *asciiStrng, UINT8 *hexBuf);
static UINT8 DecSemiOctet2Ascii(char *decSemiOctetBuf, char *asciiStrng);
static UINT8 Ascii2DecSemiOctet(char *asciiStrng, char *decSemiOctetBuf);

void Iso8859StrToGsmStr(UINT8 *cIn, UINT16 cInLen, UINT8 *gsmOut, UINT16 *gsmLen);
#if 0
static UINT8 Iso8859StrToGsmStr(char *pStrInIso, char *pStrOutGsm);
#endif

static void Gsm7BitDfltChrToIso8859Chr(char cIn, UINT8 *utf8Char, UINT8 *utf8CharLen);
static void GsmExt7BitChrToIso8859Chr(char cIn, UINT8 *utf8Char, UINT8 *utf8CharLen);
UINT16 GsmStrToIso8859Str(UINT8 *pStrInGsm, UINT8 strInGsmLen, UINT8 *pStrOutIso);

static UINT8 Text2Pdu(char *pAsciiBuf, UINT8 asciiLen, UINT8 *pPduBuf);
static UINT8 Pdu2Text(UINT8 *pPduBuf, UINT8 pduLen, char *pAsciiBuf);
//***************************************************************************
// @NAME        : Hex2Ascii
// @PARAM       : hexNibble
// @RETURNS     : Ascii Character or FALSE
// @DESCRIPTION : This function converts hex nibble to ascii character,
//				  and if fails than return FALSE.
//***************************************************************************
static UINT8 Hex2Ascii(UINT8 hexNibble)
{
	if (hexNibble <= 0x09)
	{
		return (hexNibble + 0x30);
	}
	else  if ((hexNibble >= 0x0A) && (hexNibble <= 0x0F))
	{
		return (hexNibble + 0x37);
	}
	
	return (FALSE);
}


//***************************************************************************
// @NAME        : Ascii2Hex
// @PARAM       : UINT8 asciiChar
// @RETURNS     : hex nibble or FALSE
// @DESCRIPTION : This function converts ascii pack to hex nibble.
//***************************************************************************
static UINT8 Ascii2Hex(UINT8 asciiChar)
{		
	if ((asciiChar >= '0') && (asciiChar <= '9'))
	{
		return (asciiChar - 0x30);
	}
	else  if ((asciiChar >= 'A') && (asciiChar <= 'F'))
	{
		return (asciiChar - 0x37);
	}
	
	return (FALSE);
}


//***************************************************************************
// @NAME        : HexBuf2AsciiBuf
// @PARAM       : hexBuf - Pointer to buffer containg hex data
//				  hexBufLen - length of hexBuf
// @RETURNS     : Length of Ascii string, if fails in between than returns FALSE.
// @DESCRIPTION : This function converts series of hex data in to ascii string.
//***************************************************************************
static UINT16 HexBuf2AsciiBuf(UINT8 *pHexBuf, UINT8 hexBufLen, char *pAsciiStrng)
{
	UINT8 idx = 0;
	UINT8 hexData = 0;
	UINT8 higherNibble = 0;
	UINT8 lowerNibble = 0;
	UINT8 asciiChar = 0;
	UINT16 asciiIndex = 0;
	
	for (idx =0; idx < hexBufLen; idx++)
	{
		hexData = pHexBuf[idx];
		higherNibble = hexData >> 4;
		lowerNibble = hexData & 0x0F;
		
		asciiChar = Hex2Ascii(higherNibble);
		if (asciiChar != FALSE)
		{
			pAsciiStrng[asciiIndex++] = asciiChar;
		}
		else
		{
			return (FALSE);
		}
		
		asciiChar = Hex2Ascii(lowerNibble);
		if (asciiChar != FALSE)
		{
			pAsciiStrng[asciiIndex++] = asciiChar;
		}
		else
		{
			return (FALSE);
		}
	}
	pAsciiStrng[asciiIndex] = '\0';
	
	return (asciiIndex);
}

//***************************************************************************
// @NAME        : AsciiBuf2HexBuf
// @PARAM       : asciiStrng - Pointer to buffer string buffer.
//				  hexBuf - Pointer to hex buffer.
// @RETURNS     : Length of hex buffer, if fails in between than returns FALSE.
// @DESCRIPTION : This function converts ascii string in to siries of hex data.
//***************************************************************************
static UINT8 AsciiBuf2HexBuf(char *pAsciiStrng, UINT8 *pHexBuf)
{
	UINT8 idx = 0;
	UINT8 hexData = 0;
	UINT8 higherNibble = 0;
	UINT8 lowerNibble = 0;
	UINT8 asciiChar = 0;
	UINT8 hexIndex = 0;
	UINT16 asciiLen= 0;
	
	asciiLen = strlen (pAsciiStrng);
	asciiLen = asciiLen >> 1;
	
	for (idx =0; idx < asciiLen; idx++)
	{
		/* Process higher nibble */
		asciiChar = pAsciiStrng[idx*2];
		higherNibble = Ascii2Hex(asciiChar);
		higherNibble = higherNibble << 4;
		
		/* Process lower nibble */
		asciiChar = pAsciiStrng[(idx*2)+1];
		lowerNibble = Ascii2Hex(asciiChar);
		
		/* Prepare complete hex byte */
		hexData = higherNibble | lowerNibble;
		pHexBuf[hexIndex++] = hexData;
	}
	
	return (hexIndex);
}

//***************************************************************************
// @NAME        : Iso8859StrToGsmStr
// @PARAM       : UINT8 *cIn - the pointer to buffer containing ISO-8859 characters.
//				  UINT16 cInLen - length of data in cIn buffer.
//				  UINT8 *gsmOut - The pointer to buffer which carries converetd 
//								  string in Gsm character set.
//				  UINT8 *gsmLen - The pointer to length of converetd data with 
//								  Gsm character set.
// @RETURNS     : void
// @DESCRIPTION : This function converts string in ISO-8859 character set to 
//				  Gsm cgaracter set.
//***************************************************************************
void Iso8859StrToGsmStr(UINT8 *cIn, UINT16 cInLen, UINT8 *gsmOut, UINT16 *gsmLen)
{
	UINT16 gsmIdx = 0;
	UINT16 cInidx = 0;
	UINT8 charVar = 0;
	
	
	while (cInidx < cInLen)
	{
		if (gsmIdx >= LONG_SMS_TEXT_MAX_LEN)
		{
			gsmIdx = LONG_SMS_TEXT_MAX_LEN;
			break;
		}
		
		charVar = cIn[cInidx++];
		switch(charVar)
		{
			// Characters not listed here are equal to those in the
			// ISO-8859-1 charset OR not present in it.

			case 0x40: 
				gsmOut[gsmIdx++] = 0; 
				break;
				
			case 0x24: 
				gsmOut[gsmIdx++] = 2; 
				break;			
				
			case 0x5F: 
				gsmOut[gsmIdx++] = 17; 
				break;						
				
			case 0xc2:
				switch (cIn[cInidx++])
				{
					case 0xA3:
						gsmOut[gsmIdx++] = 1; 
						break;
						
					case 0xA5:
						gsmOut[gsmIdx++] = 3; 
						break;					
						
					case 0xA4:
						gsmOut[gsmIdx++] = 36; 
						break;										
						
					case 0xA1:
						gsmOut[gsmIdx++] = 64; 
						break;															
						
					case 0xA7:
						gsmOut[gsmIdx++] = 95; 
						break;	
						
					case 0xBF:
						gsmOut[gsmIdx++] = 96; 
						break;				
						
					default:
						gsmOut[gsmIdx++] = ' '; 
						break;					
				}
				break;
				
			case 0xc3:
				switch (cIn[cInidx++])
				{
					case 0xA8:
						gsmOut[gsmIdx++] = 4; 
						break;
						
					case 0xA9:
						gsmOut[gsmIdx++] = 5; 
						break;					
						
					case 0xB9:
						gsmOut[gsmIdx++] = 6; 
						break;										
						
					case 0xAC:
						gsmOut[gsmIdx++] = 7; 
						break;															
						
					case 0xB2:
						gsmOut[gsmIdx++] = 8; 
						break;								
						
					case 0x87:
						gsmOut[gsmIdx++] = 9; 
						break;																				
						
					case 0x98:
						gsmOut[gsmIdx++] = 11; 
						break;	
						
					case 0xB8:
						gsmOut[gsmIdx++] = 12; 
						break;
						
					case 0x85:
						gsmOut[gsmIdx++] = 14; 
						break;	
						
					case 0xA5:
						gsmOut[gsmIdx++] = 15; 
						break;	
						
					case 0x86:
						gsmOut[gsmIdx++] = 28; 
						break;	
						
					case 0xA6:
						gsmOut[gsmIdx++] = 29; 
						break;					
						
					case 0x9F:
						gsmOut[gsmIdx++] = 30; 
						break;					
						
					case 0x89:
						gsmOut[gsmIdx++] = 31; 
						break;					
						
					case 0x84:
						gsmOut[gsmIdx++] = 91; 
						break;
						
					case 0x96:
						gsmOut[gsmIdx++] = 92; 
						break;										
						
					case 0x91:
						gsmOut[gsmIdx++] = 93; 
						break;										
						
					case 0x9C:
						gsmOut[gsmIdx++] = 94; 
						break;
						
					case 0xA4:
						gsmOut[gsmIdx++] = 123; 
						break;
						
					case 0xB6:
						gsmOut[gsmIdx++] = 124; 
						break;
						
					case 0xB1:
						gsmOut[gsmIdx++] = 125; 
						break;
						
					case 0xBC:
						gsmOut[gsmIdx++] = 126; 
						break;
						
					case 0xA0:
						gsmOut[gsmIdx++] = 127; 
						break;
						
					default:
						gsmOut[gsmIdx++] = ' '; 
						break;
				}
				break;	
				
				
			case 0xce:
				switch (cIn[cInidx++])
				{
					case 0x94:
						gsmOut[gsmIdx++] = 16;
						break;
						
					case 0xA6:
						gsmOut[gsmIdx++] = 18;
						break;
						
					case 0x9B:
						gsmOut[gsmIdx++] = 20;
						break;
						
					case 0xA9:
						gsmOut[gsmIdx++] = 21;
						break;
						
					case 0xA0:
						gsmOut[gsmIdx++] = 22;
						break;												
						
					case 0xA8:
						gsmOut[gsmIdx++] = 23;
						break;
						
					case 0xA3:
						gsmOut[gsmIdx++] = 24;
						break;
						
					case 0x98:
						gsmOut[gsmIdx++] = 25;
						break;
						
					case 0x9E:
						gsmOut[gsmIdx++] = 26;
						break;	
						
					default:
						gsmOut[gsmIdx++] = ' ';
						break;
				}
				break;
				
			case 0xe2:
				if ((cIn[cInidx++] == 0x82) /*&& (cIn[cInidx++] == 0x03)*/)
				{
						switch (cIn[cInidx++])
						{	
							case 0xAC:
								gsmOut[gsmIdx++] = ESC_CHR; 
								gsmOut[gsmIdx++] = 0x65; 
								break;
								
							default:
								gsmOut[gsmIdx++] = ' '; 
								break;							
						}
				}
				break;

			// extension table
			case '\f': 
				gsmOut[gsmIdx++] = ESC_CHR;
				gsmOut[gsmIdx++] = 10; 
				break; // form feed, 0x0C
				
			case '^': 
				gsmOut[gsmIdx++] = ESC_CHR;
				gsmOut[gsmIdx++] = 20; 
				break;
				
			case '{': 
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 40;
				break;
				
			case '}': 
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 41;
				break;
				
			case '\\':
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 47; 
				break;
				
			case '[': 
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 60; 
				break;
				
			case '~': 
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 61; 
				break;
				
			case ']': 
				gsmOut[gsmIdx++] = ESC_CHR;		
				gsmOut[gsmIdx++] = 62; 
				break;
				
			case '|': 
				gsmOut[gsmIdx++] = ESC_CHR;	
				gsmOut[gsmIdx++] = 64; 
				break;

			default: 
				gsmOut[gsmIdx++] = charVar;
				break;
		}
	}
	
	*gsmLen = gsmIdx;
}

//***************************************************************************
// @NAME        : Gsm7BitDfltChrToIso8859Chr
// @PARAM       : char cIn - Gsm7 bit default Character.
//				  UINT8 *utf8Char - The pointer to converted UTF character.
//				  UINT8 *utf8CharLen - The pointer to length of converted UTF 
//				 					   character.
// @RETURNS     : void
// @DESCRIPTION : This function converts Gsm 7-bit default character to UTF 
//				  characetr set.
//***************************************************************************
static void Gsm7BitDfltChrToIso8859Chr(char cIn, UINT8 *utf8Char, UINT8 *utf8CharLen)
{
	UINT8 idx = 0;
	
	switch(cIn)
	{
		// Characters not listed here are equal to those in the
		// ISO-8859-1 charset OR not present in it.

		case 0: 
			utf8Char[idx++] = 0x40; 
			break;
			
		case 1: 
			utf8Char[idx++] = 0xc2;
			utf8Char[idx++] = 0xA3;
			break;
			
		case 2:
			utf8Char[idx++] = 0x24; 
			break;
			
		case 3: 
			utf8Char[idx++] = 0xc2;	
			utf8Char[idx++] = 0xA5; 
			break;
			
		case 4: 
			utf8Char[idx++] = 0xc3;
			utf8Char[idx++] = 0xA8;
			break;
			
		case 5: 
			utf8Char[idx++] = 0xc3;
			utf8Char[idx++] = 0xA9; 
			break;
			
		case 6: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xB9; 
			break;
			
		case 7: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xAC; 
			break;
			
		case 8: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xB2; 
			break;
			
		case 9: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x87; 
			break;
			
		case 11: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x98; 
			break;
			
		case 12: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xB8; 
			break;
			
		case 14: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x85; 
			break;
			
		case 15: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xA5; 
			break;		
		
		case 16: 
			utf8Char[idx++] = 0xce; 
			utf8Char[idx++] = 0x94; 
			break;	/* Added */	
		
		case 17: 
			utf8Char[idx++] = 0x5F; 
			break;
		
		case 18: 
			utf8Char[idx++] = 0xce;
			utf8Char[idx++] = 0xA6; 
			break;	/* Added */	
			
		case 19: 
			/* Pending */
			break;	/* Added */	
			
		case 20: 
			utf8Char[idx++] = 0xce; 	
			utf8Char[idx++] = 0x9B;
			break;	/* Added */	
			
		case 21: 
			utf8Char[idx++] = 0xce;
			utf8Char[idx++] = 0xA9; 
			break;	/* Added */	
			
		case 22: 
			utf8Char[idx++] = 0xce;
			utf8Char[idx++] = 0xA0; 
			break;	/* Added */	
			
		case 23: 
			utf8Char[idx++] = 0xce; 
			utf8Char[idx++] = 0xA8; 
			break;	/* Added */	
			
		case 24: 
			utf8Char[idx++] = 0xce; 
			utf8Char[idx++] = 0xA3; 
			break;	/* Added */	
			
		case 25: 
			utf8Char[idx++] = 0xce;
			utf8Char[idx++] = 0x98; 
			break;	/* Added */	
		case 26: 
			utf8Char[idx++] = 0xce; 
			utf8Char[idx++] = 0x9E; 
			break;	/* Added */	
		
		case 28: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x86; 			
			break;
			
		case 29: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xA6; 			
			break;
			
		case 30: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x9F; 			
			break;
			
		case 31: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x89; 			
			break;
			
		case 36: 
			utf8Char[idx++] = 0xc2; 
			utf8Char[idx++] = 0xA4; 
			break; // 164 in ISO-8859-1
			
		case 64: 
			utf8Char[idx++] = 0xc2; 
			utf8Char[idx++] = 0xA1; 
			break;
			
		// 65-90 capital letters
		case 91: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x84; 
			break;
			
		case 92: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x96; 			
			break;
			
		case 93: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x91; 						
			break;
			
		case 94: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0x9C; 						
			break;
			
		case 95: 
			utf8Char[idx++] = 0xc2; 
			utf8Char[idx++] = 0xA7; 						
			break;
			
		case 96:
			utf8Char[idx++] = 0xc2; 
			utf8Char[idx++] = 0xBF; 									
			break;
			
		// 97-122 small letters
		case 123: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xA4;
			break;
			
		case 124: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xB6;			
			break;
			
		case 125: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xB1;				
			break;
			
		case 126: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xBC;							
			break;
			
		case 127: 
			utf8Char[idx++] = 0xc3; 
			utf8Char[idx++] = 0xA0;
			break;

		default: 
			utf8Char[idx++] = cIn; 
			break;
	}
	
	*utf8CharLen = idx;
}

//***************************************************************************
// @NAME        : GsmExt7BitChrToIso8859Chr
// @PARAM       : char cIn - Gsm extended 7-bit default Character.
//				  UINT8 *utf8Char - The pointer to converted UTF character.
//				  UINT8 *utf8CharLen - The pointer to length of converted UTF 
//				 					   character.
// @RETURNS     : void
// @DESCRIPTION : This function converts Gsm extended 7-bit character to UTF 
//				  characetr set.
//***************************************************************************
static void GsmExt7BitChrToIso8859Chr(char cIn, UINT8 *utf8Char, UINT8 *utf8CharLen)
{
	UINT8 idx = 0;
	
	switch(cIn)
	{
		// Characters not listed here are equal to those in the
		// ISO-8859-1 charset OR not present in it.

		// extension table
		case 10: 
			utf8Char[idx++] = '\f'; 
			break; // form feed, 0x0C
			
		case 20: 
			utf8Char[idx++] = '^';  
			break;
			
		case 40: 
			utf8Char[idx++] = '{';  
			break;
			
		case 41: 
			utf8Char[idx++] = '}';  
			break;
			
		case 47:
			utf8Char[idx++] = '\\';  
			break;
			
		case 60: 
			utf8Char[idx++] = '[';  
			break;
			
		case 61: 
			utf8Char[idx++] = '~';  
			break;
			
		case 62: 
			utf8Char[idx++] = ']';  
			break;
			
		case 64: 
			utf8Char[idx++] = '|';  
			break;
			
		case 101: 
			utf8Char[idx++] = 0xe2;  
			utf8Char[idx++] = 0x82;  
			utf8Char[idx++] = 0xAC;  
			break; // 164 in ISO-8859-15

		default: 
			utf8Char[idx++] = cIn; 
			break;
	}
	
	*utf8CharLen = idx;
}

//***************************************************************************
// @NAME        : GsmStrToIso8859Str
// @PARAM       : UINT8 *pStrInGsm - The pointer to string with Gsm character set.
//				  UINT8 strInGsmLen - length of string with gsm character set..
//				  UINT8 *pStrOutIso - The pointer to buffer containing converted 
//									  ISO-8859 character set.
// @RETURNS     : void
// @DESCRIPTION : This function converts Gsm 7-bit characters to ISO-8859 characters.
//***************************************************************************
UINT16 GsmStrToIso8859Str(UINT8 *pStrInGsm, UINT8 strInGsmLen, UINT8 *pStrOutIso)
{
	UINT8 index = 0;
	UINT8 utf8CharLen = 0;
	UINT16 cnvrtdStrIndex = 0;	
	
	for (index = 0; index < strInGsmLen; index++)
	{
		if (pStrInGsm[index] == ESC_CHR)
		{
			index++;
			GsmExt7BitChrToIso8859Chr(pStrInGsm[index], &pStrOutIso[cnvrtdStrIndex], &utf8CharLen);
			cnvrtdStrIndex += utf8CharLen;
		}
		else
		{
			Gsm7BitDfltChrToIso8859Chr(pStrInGsm[index], &pStrOutIso[cnvrtdStrIndex], &utf8CharLen);
			cnvrtdStrIndex += utf8CharLen;
		}
	}
	
	if (cnvrtdStrIndex > 550)
	{
		cnvrtdStrIndex = 550;
	}
	pStrOutIso[cnvrtdStrIndex] = '\0';	
	
	return (cnvrtdStrIndex);
}

#if 0
static UINT8 Iso8859StrToGsmStr(char *pStrInIso, char *pStrOutGsm)
{
	UINT8 index = 0;
	UINT8 strLen = 0;
	char cnvrtdChr;
	UINT8 cnvrtdStrIndex = 0;
	
	strLen = strlen(pStrInIso);
	for (index = 0; index < strLen; index++)
	{
		if (Iso8859ChrToGsm7BitDfltChr(pStrInIso[index], &cnvrtdChr) == TRUE)
		{
			pStrOutGsm[cnvrtdStrIndex++] = ESC_CHR;
			pStrOutGsm[cnvrtdStrIndex++] = cnvrtdChr;
		}
		else
		{
			pStrOutGsm[cnvrtdStrIndex++] = cnvrtdChr;
		}
	}
	
	if (cnvrtdStrIndex > SMS_TEXT_MAX_LEN)
	{
		cnvrtdStrIndex = SMS_TEXT_MAX_LEN;
	}
	pStrOutGsm[cnvrtdStrIndex] = '\0';
	
	return(cnvrtdStrIndex);
}
#endif
//***************************************************************************
// @NAME        : DecSemiOctet2Ascii
// @PARAM       : decSemiOctetBuf - Pointer to decimal semi octet buffer.
//				  asciiStrng - Pointer to ascii buffer.
// @RETURNS     : void.
// @DESCRIPTION : This function converts decimal semi octet in to ascii.
//***************************************************************************
static UINT8 DecSemiOctet2Ascii(char *pDecSemiOctetBuf, char *pAsciiStrng)
{
	UINT8 idx = 0;
	UINT8 asciiChar = 0;
	UINT8 asciiNextChar = 0;
	UINT8 decSemiOctetLen= 0;	
	UINT8 loopCnt = 0;
	
	decSemiOctetLen = strlen (pDecSemiOctetBuf);
	loopCnt = decSemiOctetLen >> 1;
	
	for (idx = 0; idx < loopCnt; idx++)
	{
		asciiChar = pDecSemiOctetBuf[idx*2];
		asciiNextChar = pDecSemiOctetBuf[(idx*2) + 1];
		
		pAsciiStrng[idx*2] = asciiNextChar;
		if (idx == (loopCnt - 1))
		{
			if (asciiChar == 'F')
			{
				pAsciiStrng[(idx*2) + 1] = '\0';
				return (decSemiOctetLen - 1);
			}
			else
			{
				pAsciiStrng[(idx*2) + 1] = asciiChar;
			}
		}
		else
		{
			pAsciiStrng[(idx*2) + 1] = asciiChar;
		}
	}
	
	pAsciiStrng[idx*2] = '\0';
	
	return (decSemiOctetLen);
}


//***************************************************************************
// @NAME        : Ascii2DecSemiOctet
// @PARAM       : asciiStrng - Pointer to ascii buffer.
//				  decSemiOctetBuf - Pointer to decimal semi octet buffer.
// @RETURNS     : void.
// @DESCRIPTION : This function converts ascii in to decimal semi octet.
//***************************************************************************
static UINT8 Ascii2DecSemiOctet(char *pAsciiStrng, char *pDecSemiOctetBuf)
{
	UINT8 idx = 0;
	UINT8 asciiChar = 0;
	UINT8 asciiNextChar = 0;
	UINT8 asciiLen= 0;	
	UINT8 loopCnt = 0;
	UINT8 isToAddF = 0;
	
	asciiLen = strlen (pAsciiStrng);
	if ((asciiLen % 2) != 0)
	{
		asciiLen++;
		isToAddF = TRUE;
	}
	loopCnt = asciiLen >> 1;
	
	for (idx = 0; idx < loopCnt; idx++)
	{
		asciiChar = pAsciiStrng[idx*2];
		if (idx == (loopCnt - 1))
		{
			if (isToAddF == TRUE)
			{
				asciiNextChar = 'F';
			}
			else
			{
				asciiNextChar = pAsciiStrng[(idx*2) + 1];
			}
		}
		else
		{
			asciiNextChar = pAsciiStrng[(idx*2) + 1];
		}
		
		pDecSemiOctetBuf[idx*2] = asciiNextChar;
		pDecSemiOctetBuf[(idx*2) + 1] = asciiChar;
	}
	pDecSemiOctetBuf[idx*2] = '\0';
	
	return (asciiLen);
}


//***************************************************************************
// @NAME        : TextToPdu
// @PARAM       : asciiBuf- Pointer to ascii buffer containing text data.
//				  pduBuf - Pointer to pdu buffer(for converted pdu data).
// @RETURNS     : UINT8,pdu data length.
// @DESCRIPTION : This function converts Text data to pdu data.
//***************************************************************************
static UINT8 Text2Pdu(char *pAsciiBuf, UINT8 asciiLen, UINT8 *pPduBuf)
{
	UINT8 idx = 0;
	UINT8 index = 0;
	UINT8 pduIndex = 0;
	UINT8 procChar = 0;
	UINT8 procNextChar = 0;
	UINT8 procVariable = 0;
	UINT8 tempVariable;
	UINT8 asciiChar = 0;
	
	for (idx = 0; idx < asciiLen; idx++)
	{
		asciiChar = pAsciiBuf[idx];
		procChar = asciiChar >> index;
		
		if (index != 7)
		{
			procNextChar = pAsciiBuf[idx + 1];
			tempVariable = procNextChar;
			procVariable = 0xFF << (index + 1);
			tempVariable = tempVariable & (~procVariable);
			tempVariable = tempVariable << (7 - index);
			pPduBuf[pduIndex++] = procChar | tempVariable;
		}

		index++;
		if (index > 7)	index = 0;
	}
	
	return (pduIndex);
}

//***************************************************************************
// @NAME        : PduToText
// @PARAM       : pduBuf - Pointer to pdu buffer(for converted pdu data).
//				: pduLen - length of pdu data.
//				  asciiBuf- Pointer to ascii buffer containing text data.
// @RETURNS     : UINT8,pdu data length.
// @DESCRIPTION : This function converts pdu data to text data.
//***************************************************************************
static UINT8 Pdu2Text(UINT8 *pPduBuf, UINT8 pduLen, char *pAsciiBuf)
{
	UINT8 idx = 0;
	UINT8 index = 0;
	UINT8 asciiIndex = 0;
	UINT8 procByte = 0;
	UINT8 procPrevByte = 0;
	UINT8 procVariable = 0;
	UINT8 tempVariable = 0;
	UINT8 pduByte = 0;
	
	
	for (idx = 0; idx < pduLen; idx++)
	{
		pduByte = pPduBuf[idx];
		procByte = pduByte & (0xFF >> (index + 1));
		procByte = procByte << index;
		if (idx > 0)
		{
			procPrevByte = pPduBuf[idx - 1];
			tempVariable = procPrevByte;
			procVariable = 0xFF >> index;
			tempVariable = tempVariable & (~procVariable);
			tempVariable = tempVariable >> (7 - (index - 1));
		}
		else
		{
			tempVariable = 0;
		}
		
		pAsciiBuf[asciiIndex++] = procByte | tempVariable;
		if (index == 6)
		{
			if (idx == (pduLen - 1))
			{
				if ((pduByte >> 1) != 0)
				{
					pAsciiBuf[asciiIndex++] = pduByte >> 1;
				}
			}
			else
			{
				pAsciiBuf[asciiIndex++] = pduByte >> 1;
			}
		}
		
		index++;
		if (index  > 6)		index = 0;
	}
	
	pAsciiBuf[asciiIndex] = '\0';
	
	return (asciiIndex);
}

//***************************************************************************
// @NAME        : GsmLib_FramePduDataFrmt
// @PARAM       : 
//				: 
// @RETURNS     : 
// @DESCRIPTION : It frame PDU SMS in pdu format
//***************************************************************************
UINT16 GsmLib_FramePduDataFrmt(PDU_FRAME_DESC *pPduFrameDesc)
{
	static UINT8 refferenceNum = 0;
	
	UINT8 firstSmsOctet = 0;
	UINT8 typeOfAddr = 0;
	char desAddrBuf[ADDR_MAX_LEN + 1];
	UINT8 pduHexBuf[SMS_PDU_MAX_LEN + 1];
	UINT8 pduHexLen = 0;
	char textStrng[(SMS_PDU_MAX_LEN * 2) + 1];
	UINT16 sendSmsBufLen = 0;
	
	char procText[SMS_TEXT_MAX_LEN*2];
	UINT8 procTextLen = 0;
	
	/* UDH related variable */
	UINT8 udl = 0; // User Data Length
	UINT8 udhl = UDH_CONCATENATED_MSG_LEN; // User Data Header Length/
	
	memset(procText, '\0', sizeof(procText));
	
	/* Indicate to obtain stored SCA - 0x00 */
	
	/* First octet of SMS-SUBMIT message */
	firstSmsOctet |= 0x01;	 // Indicate SMS-SUBMIT request
	firstSmsOctet |= 0x10;	 // Validity period - one octet
	/* Only set status report indication bit if it is last part of concatenated SMS */
	if (pPduFrameDesc->isConcatenatedSms == TRUE)
	{
		firstSmsOctet |= pPduFrameDesc->isConcatenatedSms << 6;	// Indicate that UDH is present
		/* Is last part to send? */
		if (pPduFrameDesc->totalParts == pPduFrameDesc->partNum) // Yes
		{
			/* Indicate that delivery report is require */
			firstSmsOctet |= pPduFrameDesc->isDlvryRprtEnble << 5;	// Indicate that delivery report is require
		}
	}
	/* set status report indication bit */
	else
	{
		firstSmsOctet |= pPduFrameDesc->isDlvryRprtEnble << 5;	// Indicate that delivery report is require
	}
	
	
	/* Message Reference - let phone set the message reference itself. */
	/* 0x00 */

	/* Address Length- length of phone number */
		
	/* Type of address */
	if (pPduFrameDesc->isIntrNtnlNum == TRUE)
	{
		/* International format */
		typeOfAddr = 0x91;
	}
	else
	{
		/* National format */
		typeOfAddr = 0x81;
	}
	
	/* Converts Ascii to decimal semi octet */
	Ascii2DecSemiOctet(pPduFrameDesc->pDestAddr, desAddrBuf);

	/* Protocol Identifier - 0x00 */
	
	/* Data coding scheme - 0x00 */
	
	/* Validity period - 0x17 */	//Approx two hours - TODO decide
	
	/* Length of Text data */
	
	/* User data field */
	
	/* Is concatenated SMS */
	if (pPduFrameDesc->isConcatenatedSms == TRUE) // Yes
	{
		UINT8 fillBitsNum = 0;
		UINT8 udhSeptet = 0;
		
		/* Derive number of octet to be filled */
		fillBitsNum = ((1 + udhl) * 8) % 7;
		fillBitsNum = 7 - fillBitsNum;
		
		/* Derive User Data Length in septets */
		udl = (((1 + udhl) * 8) + fillBitsNum) / 7;
		udhSeptet = udl;
		udl += pPduFrameDesc->textLen;
		
		/* Frame PDU data up to Concatenated SMS part number */
		sprintf(pPduFrameDesc->pPduFrmBuf,"%02X%02X%02X%02X%02X%s%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"
		, 0, firstSmsOctet, 0, strlen(pPduFrameDesc->pDestAddr), typeOfAddr, desAddrBuf, 0, 0, pPduFrameDesc->vp,  
		udl , udhl, IE_CONCATENATED_MSG, IE_CONCATENATED_MSG_LEN, 
		refferenceNum, pPduFrameDesc->totalParts, pPduFrameDesc->partNum);
		
		/* Copy text to intermediate buffer */
		procTextLen = pPduFrameDesc->textLen;
		memcpy(&procText[udhSeptet], pPduFrameDesc->pTextMsg, procTextLen);		
		
		/* Converts text data to pdu data */
		pduHexLen = Text2Pdu(procText, procTextLen + udhSeptet, pduHexBuf);
		HexBuf2AsciiBuf(&pduHexBuf[1 + udhl], pduHexLen - (1 + udhl), textStrng);
		
		/* Concanet pdu text */
		strcat(pPduFrameDesc->pPduFrmBuf, textStrng);
	}
	else // No
	{
		/* Copy text to intermediate buffer */
		procTextLen = pPduFrameDesc->textLen;
		memcpy(procText, pPduFrameDesc->pTextMsg, procTextLen);
		
		/* Converts text data to pdu data */
		pduHexLen = Text2Pdu(procText, procTextLen, pduHexBuf);
		HexBuf2AsciiBuf(pduHexBuf, pduHexLen, textStrng);
		
		sprintf(pPduFrameDesc->pPduFrmBuf,"%02X%02X%02X%02X%02X%s%02X%02X%02X%02X%s"
		, 0, firstSmsOctet, 0, strlen(pPduFrameDesc->pDestAddr), typeOfAddr, desAddrBuf, 0, 0, pPduFrameDesc->vp, pPduFrameDesc->textLen, textStrng);		
	}
	
	/* Is last part sent? */
	if (pPduFrameDesc->totalParts == pPduFrameDesc->partNum) // Yes
	{
		/* Increment reffrence number */
		refferenceNum++;
	}	
		
	/* Deriving length to be sent in AT command */
	sendSmsBufLen = strlen(pPduFrameDesc->pPduFrmBuf);
	
	return (sendSmsBufLen);
}

//***************************************************************************
// @NAME        : DecodeSmsDataFrmt
// @PARAM       : 
// @RETURNS     : void
// @DESCRIPTION : It decode SMS in pdu format to text format
//***************************************************************************
void GsmLib_DecodeSmsSubmitPduFrmt(PDU_DECODE_DESC *pPduDecodeDesc)
{
	UINT16 idx = 0;
	UINT8 scaLen = 0;
	UINT8 frstOctet = 0;
	UINT8 addrType = 0;
	UINT8 dcs = 0;
	BOOL udhi = 0;
	UINT8 udhiLen = 0;
	UINT8 fillBits = 0;
	
	UINT8 oaLen;
	char oaBuf[ADDR_MAX_LEN + 1];   /* Oraginating address */
	char timeStampBuf[TIME_STAMP_LEN + 1];	 /* Time STamp of SMS */
	char pduAsciiData[(SMS_PDU_MAX_LEN * 2) + 1];  /* Buffer for PDU data */
	
	UINT8 pduHexBuf[SMS_PDU_MAX_LEN + 1];  /* Buffer for Hex Data */
	UINT8 pduHexLen = 0;
	
	UINT8 textLen = 0;
	
	/* Service center number length */
	scaLen = Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	scaLen = scaLen << 4;
	scaLen = scaLen | Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	
	/* Skip Service Center Number */
	idx = idx + (scaLen * 2);
	
	/* ignore first octet of sms deliver message */
	frstOctet = Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	frstOctet = frstOctet << 4;
	frstOctet = frstOctet | Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	
	if ((frstOctet & 0x40) == 0x40)
	{
		udhi = TRUE;
	}
	/* Oraginating adress length */
	oaLen = Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	oaLen = oaLen << 4;
	oaLen = oaLen | Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	if ((oaLen % 2) != 0)
	{
		/* Eg: For "46708251358" Number Length will be 11 ("6407281553F8") */
		/* Eg: For "9898907249" Number Length will be 12 ("8989092794") */
		oaLen++;
	}
	
	/* Type of address (Eg: 91 , 81)*/
	addrType = Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	addrType = addrType << 4;
	addrType = addrType | Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);	
	pPduDecodeDesc->typeOfAddr = addrType;
	
	/* Oraginating address */
	memcpy(oaBuf, &pPduDecodeDesc->pSmsBuf[idx], oaLen);
	oaBuf[oaLen] = '\0';
	if ((addrType & 0x70) == ALPHANUMERIC_NUM_PLAN)
	{
		memcpy(pduAsciiData, oaBuf, oaLen);
		pduAsciiData[oaLen] = '\0';
	
		pduHexLen = AsciiBuf2HexBuf(pduAsciiData, pduHexBuf);
		textLen = Pdu2Text(pduHexBuf, pduHexLen, oaBuf);
		/* Map Character to Non printable Character */
		memcpy(pPduDecodeDesc->pOrgAddr, oaBuf, textLen);
		pPduDecodeDesc->orgAddrLen = textLen;
		//GsmStrToIso8859Str(oaBuf, textLen, pOrgAddr);
	}
	else
	{
		DecSemiOctet2Ascii(oaBuf, pPduDecodeDesc->pOrgAddr);
		pPduDecodeDesc->orgAddrLen = strlen(pPduDecodeDesc->pOrgAddr);
	}
	idx = idx + oaLen;

	
	/* Skip Protocol Identifier */
	idx = idx + 2;
	
	/* Skip Data coding scheme */
	dcs = Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	dcs = addrType << 4;
	dcs = dcs | Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	
	/* Time stamp */
	memcpy(timeStampBuf, &pPduDecodeDesc->pSmsBuf[idx], TIME_STAMP_LEN);
	timeStampBuf[TIME_STAMP_LEN] = '\0';
	DecSemiOctet2Ascii(timeStampBuf, pPduDecodeDesc->pTimeStamp);
	idx = idx + TIME_STAMP_LEN;	
	
	/* Length of text message in terms of characters  OR octet */
	idx = idx + 2;	
		
	switch (dcs & 0xC0)
	{	
		case UTF7:
			/* Skip UDHI */
			if (udhi == TRUE)
			{		
				UINT8 tmpIdx = 0;
				
				tmpIdx = idx;
				udhiLen = Ascii2Hex(pPduDecodeDesc->pSmsBuf[tmpIdx++]);
				udhiLen = udhiLen << 4;
				udhiLen = udhiLen | Ascii2Hex(pPduDecodeDesc->pSmsBuf[tmpIdx++]);
				
				fillBits = ((udhiLen+1)*8) % 7;
				fillBits = 7 - fillBits;
				fillBits = ((udhiLen+1)*8) + fillBits;
				fillBits = fillBits / 7;
			}	
			
			/* pdu data to ascii */
			memcpy(pduAsciiData, &pPduDecodeDesc->pSmsBuf[idx], pPduDecodeDesc->smsBufLen - idx);
			pduAsciiData[pPduDecodeDesc->smsBufLen - idx] = '\0';
			
			pduHexLen = AsciiBuf2HexBuf(pduAsciiData, pduHexBuf);			
			textLen = Pdu2Text(pduHexBuf, pduHexLen, pduAsciiData);
			
			if (udhi == TRUE)
			{
				textLen = textLen - fillBits;
				memcpy(pduAsciiData, &pduAsciiData[fillBits], textLen );
				pduAsciiData[textLen] = '\0';
			}
			
			memcpy((char *)pPduDecodeDesc->pTextMsg, pduAsciiData, textLen);
			pPduDecodeDesc->textMsgLen = textLen;
			break;
			
		case UTF8:
			/* Skip UDHI */
			if (udhi == TRUE)
			{						
				udhiLen = Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
				udhiLen = udhiLen << 4;
				udhiLen = udhiLen | Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
				
				idx = idx + (udhiLen * 2);
			}			
		
			memcpy(pPduDecodeDesc->pTextMsg, &pPduDecodeDesc->pSmsBuf[idx], pPduDecodeDesc->smsBufLen - idx);
			pPduDecodeDesc->pTextMsg[pPduDecodeDesc->smsBufLen - idx] = '\0';
			pPduDecodeDesc->textMsgLen = pPduDecodeDesc->smsBufLen - idx;
			break;
			
		case UTF16:
			/* Not Implemented */
			break;
			
		default:
			/* Resereved */
			break;
	}
}


//***************************************************************************
// @NAME        : DecodeSmsDlvryRprtDataFrmt
// @PARAM       : 
// @RETURNS     : void
// @DESCRIPTION : It decode SMS in pdu format to text format
//***************************************************************************
void GsmLib_DecodeSmsDlvryRprtPduFrmt(PDU_DECODE_DESC *pPduDecodeDesc)
{
	UINT16 idx = 0;
	UINT8 scaLen = 0;
	UINT8 frstOctet = 0;
	UINT8 addrType = 0;
	
	UINT8 oaLen;
	char oaBuf[ADDR_MAX_LEN+1]; /* Oraginating address */	
	char timeStampBuf[TIME_STAMP_LEN + 1];
	
	
	/* Service center number length */
	scaLen = Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	scaLen = scaLen << 4;
	scaLen = scaLen | Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	idx = idx + (scaLen * 2);
	
	/* ignore first octet of sms deliver message */
	frstOctet = Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	frstOctet = frstOctet << 4;
	frstOctet = frstOctet | Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);

	/* Skip Message Reference Number */
	idx = idx + 2;
	
	/* Oraginating adress length */
	oaLen = Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	oaLen = oaLen << 4;
	oaLen = oaLen | Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	if ((oaLen % 2) != 0)
	{
		oaLen++;
	}
	
	/* Type of address (Eg: 91 , 81)*/
	addrType = Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);
	addrType = addrType << 4;
	addrType = addrType | Ascii2Hex(pPduDecodeDesc->pSmsBuf[idx++]);	
	pPduDecodeDesc->typeOfAddr = addrType;
	
	/* Oraginating address */
	memcpy(oaBuf, &pPduDecodeDesc->pSmsBuf[idx], oaLen);
	oaBuf[oaLen] = '\0';
	DecSemiOctet2Ascii(oaBuf, pPduDecodeDesc->pOrgAddr);
	idx = idx + oaLen;
		
	/* Time stamp */
	memcpy(timeStampBuf, &pPduDecodeDesc->pSmsBuf[idx], TIME_STAMP_LEN);
	timeStampBuf[TIME_STAMP_LEN] = '\0';
	DecSemiOctet2Ascii(timeStampBuf, pPduDecodeDesc->pTimeStamp);
	idx = idx + TIME_STAMP_LEN;	
}


