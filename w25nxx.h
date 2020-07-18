#ifndef _W25NXX_H
#define _W25NXX_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>
#include "stm32f10x_spi.h"

#define _W25NXX_SPI SPI2
#define _W25NXX_CS_GPIO SPI2_PORT //You should define spi port before
#define _W25NXX_CS_PIN SPI2_nCS_MEM //You should define spi cs pin before
#define _W25NXX_DEBUG 0

#define CMD_DEVICE_RESET 0xFF
#define CMD_JDEC_ID 0x9F
#define CMD_READ_STATUS_R 0x0F
#define CMD_WRITE_STATUS_R 0x1
#define CMD_WRITE_ENABLE 0x06
#define CMD_WRITE_DISABLE 0x04
#define CMD_BLOCK_ERASE 0xD8
#define CMD_PAGE_DATA_READ 0x13
#define CMD_READ 0x03
#define CMD_FAST_READ 0x0B
#define CMD_DATA_LOAD 0x02
#define CMD_PROGRAM_EXEC 0x10

#define ADDR_STATUS_R_1 0xA0
#define ADDR_STATUS_R_2 0xB0
#define ADDR_STATUS_R_3 0xC0
	typedef enum
	{
		W25N10 = 1,
		W25N20,
		W25N40,
		W25N80,
		W25N16,
		W25N32,
		W25N64,
		W25N128,
		W25N256,
		W25N512,

	} W25NXX_ID_t;

	typedef struct
	{
		W25NXX_ID_t ID;
		uint8_t UniqID[8];
		uint16_t PageSize;
		uint32_t PageCount;
		uint32_t SectorSize;
		uint32_t SectorCount;
		uint32_t BlockSize;
		uint32_t BlockCount;
		uint32_t CapacityInKiloByte;
		uint8_t StatusRegister1;
		uint8_t StatusRegister2;
		uint8_t StatusRegister3;
		uint8_t Lock;

	} W25Nxx_t;

	extern W25Nxx_t W25Nxx;
	//############################################################################
	// in Page,Sector and block read/write functions, can put 0 to read maximum bytes
	//############################################################################
	bool W25Nxx_Init(void);

	void W25Nxx_EraseChip(void);
	void W25Nxx_EraseSector(uint32_t SectorAddr);
	void W25Nxx_EraseBlock(uint32_t BlockAddr);

	uint32_t W25Nxx_PageToSector(uint32_t PageAddress);
	uint32_t W25Nxx_PageToBlock(uint32_t PageAddress);
	uint32_t W25Nxx_SectorToBlock(uint32_t SectorAddress);
	uint32_t W25Nxx_SectorToPage(uint32_t SectorAddress);
	uint32_t W25Nxx_BlockToPage(uint32_t BlockAddress);

	bool W25Nxx_IsEmptyPage(uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize);
	bool W25Nxx_IsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize);
	bool W25Nxx_IsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize);

	void W25Nxx_WriteByte(uint8_t pBuffer, uint32_t Bytes_Address);
	void W25Nxx_WritePage(uint8_t *pBuffer,uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize);
	void W25Nxx_WriteSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize);
	void W25Nxx_WriteBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);
	void W25Nxx_WriteStatusRegister(uint8_t SelectStatusRegister_1_2_3, uint8_t Data);
	void W25Nxx_Execute(uint32_t Page);

	void W25Nxx_ReadByte(uint8_t *pBuffer, uint32_t Bytes_Address);
	void W25Nxx_ReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);
	void W25Nxx_ReadPage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize);
	void W25Nxx_ReadSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize);
	void W25Nxx_ReadBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);
//############################################################################
#ifdef __cplusplus
}
#endif

#endif
