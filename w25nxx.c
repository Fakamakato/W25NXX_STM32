
#include "W25Nxx.h"

#if (_W25NXX_DEBUG == 1)
#include <stdio.h>
#endif

#define W25NXX_DUMMY_BYTE 0xFF
#define BUF_SIZE 2048

W25Nxx_t W25Nxx;
//These two functions below must be defined somewhere else
extern uint32_t Get_Time_Stamp(void); // returns value of a variable that increments every millisecond
extern void Reset_Time_Stamp(void);   // resets the variable

void W25Nxx_Delay(int delay) //milliseconds
{
	Reset_Time_Stamp();
	while (delay > Get_Time_Stamp())
		;
}
//###################################################################################################################
uint8_t W25Nxx_Spi(uint8_t Data)
{
	uint8_t ret;
	SPI_I2S_SendData(_W25NXX_SPI, Data);
	while (SPI_I2S_GetFlagStatus(_W25NXX_SPI, SPI_I2S_FLAG_BSY) == SET)
		;
	ret = SPI_I2S_ReceiveData(_W25NXX_SPI);
	return ret;
}
//###################################################################################################################
uint32_t W25Nxx_ReadID(void)
{
	uint32_t Temp = 0;
	_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
	W25Nxx_Spi(CMD_JDEC_ID);
	W25Nxx_Spi(W25NXX_DUMMY_BYTE);
	Temp += W25Nxx_Spi(W25NXX_DUMMY_BYTE) << 16;
	Temp += W25Nxx_Spi(W25NXX_DUMMY_BYTE) << 8;
	Temp += W25Nxx_Spi(W25NXX_DUMMY_BYTE);
	_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
	return Temp;
}
//###################################################################################################################
void W25Nxx_ReadUniqID(void)
{
	_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
	W25Nxx_Spi(0x4B);
	for (uint8_t i = 0; i < 4; i++)
		W25Nxx_Spi(W25NXX_DUMMY_BYTE);
	for (uint8_t i = 0; i < 8; i++)
		W25Nxx.UniqID[i] = W25Nxx_Spi(W25NXX_DUMMY_BYTE);
	_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
	;
}
//###################################################################################################################
void W25Nxx_WriteEnable(void)
{
	_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
	W25Nxx_Spi(CMD_WRITE_ENABLE);
	_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
	;
	W25Nxx_Delay(1);
}
//###################################################################################################################
void W25Nxx_WriteDisable(void)
{
	_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
	W25Nxx_Spi(CMD_WRITE_DISABLE);
	_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
	;
	W25Nxx_Delay(1);
}
//###################################################################################################################
uint8_t W25Nxx_ReadStatusRegister(uint8_t SelectStatusRegister_1_2_3)
{
	uint8_t status = 0;
	_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
	W25Nxx_Spi(CMD_READ_STATUS_R);
	if (SelectStatusRegister_1_2_3 == 1)
	{

		W25Nxx_Spi(ADDR_STATUS_R_1);
		status = W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		W25Nxx.StatusRegister1 = status;
	}
	else if (SelectStatusRegister_1_2_3 == 2)
	{
		W25Nxx_Spi(ADDR_STATUS_R_2);
		status = W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		W25Nxx.StatusRegister2 = status;
	}
	else
	{
		W25Nxx_Spi(ADDR_STATUS_R_3);
		status = W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		W25Nxx.StatusRegister3 = status;
	}
	_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
	;
	return status;
}
//###################################################################################################################
void W25Nxx_WriteStatusRegister(uint8_t SelectStatusRegister_1_2_3, uint8_t Data)
{
	_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
	W25Nxx_Spi(CMD_WRITE_STATUS_R);
	if (SelectStatusRegister_1_2_3 == 1)
	{
		W25Nxx_Spi(ADDR_STATUS_R_1);
		W25Nxx.StatusRegister1 = Data;
	}
	else if (SelectStatusRegister_1_2_3 == 2)
	{
		W25Nxx_Spi(ADDR_STATUS_R_2);
		W25Nxx.StatusRegister2 = Data;
	}
	else
	{
		W25Nxx_Spi(ADDR_STATUS_R_3);
		W25Nxx.StatusRegister3 = Data;
	}
	W25Nxx_Spi(Data);
	_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
	;
}
//###################################################################################################################
void W25Nxx_WaitForWriteEnd(void)
{
	W25Nxx_Delay(1);
	_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
	W25Nxx_Spi(CMD_READ_STATUS_R);
	do
	{

		W25Nxx.StatusRegister3 = W25Nxx_ReadStatusRegister(3);
		W25Nxx_Delay(1);
	} while ((W25Nxx.StatusRegister3 & 0x01) == 0x01);
	_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
	;
}
//###################################################################################################################
void W25Nxx_Reset(void)
{
	W25Nxx_Delay(1);
	_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
	W25Nxx_Spi(CMD_DEVICE_RESET);
	_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
}
//###################################################################################################################
bool W25Nxx_Init(void)
{
	W25Nxx.Lock = 1;
	GPIOB->BSRR = FLASH_WP;
	_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
	W25Nxx_Delay(50);
	W25Nxx_Reset();
	W25Nxx_Delay(100);
	uint32_t id;
	id = W25Nxx_ReadID();
	W25Nxx.ID = 10;
	W25Nxx.BlockCount = 1024;
	W25Nxx.PageSize = 2048;
	W25Nxx.SectorSize = 512;
	W25Nxx.SectorCount = W25Nxx.BlockCount * 4;
	W25Nxx.PageCount = (W25Nxx.SectorCount * W25Nxx.SectorSize) / W25Nxx.PageSize;
	W25Nxx.BlockSize = W25Nxx.PageCount * 64;
	W25Nxx.CapacityInKiloByte = 125000;
	W25Nxx_ReadStatusRegister(1);
	W25Nxx_ReadStatusRegister(2);
	W25Nxx_ReadStatusRegister(3);
	W25Nxx_WriteStatusRegister(1,0);
	W25Nxx.Lock = 0;
	return true;
}
//###################################################################################################################
void W25Nxx_EraseChip(void)
{
	for (unsigned int i = 0; i < W25Nxx.BlockCount; i++)
	{
		uint16_t addr = i * 64;
		W25Nxx_WriteEnable();
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(CMD_BLOCK_ERASE);
		W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		W25Nxx_Spi(addr >> 8); //page address
		W25Nxx_Spi(addr && 0x00FF);
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		W25Nxx_WaitForWriteEnd();
		W25Nxx_Delay(10);
		W25Nxx.Lock = 0;
	}
}
	//###################################################################################################################
	/*
void W25Nxx_EraseSector(uint32_t SectorAddr)
{
	while (W25Nxx.Lock == 1)
		W25Nxx_Delay(1);
	W25Nxx.Lock = 1;
#if (_W25NXX_DEBUG == 1)
	uint32_t StartTime = Get_Time_Stamp();
	printf("W25Nxx EraseSector %d Begin...\r\n", SectorAddr);
#endif
	W25Nxx_WaitForWriteEnd();
	SectorAddr = SectorAddr * W25Nxx.SectorSize;
	W25Nxx_WriteEnable();
	_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
	W25Nxx_Spi(0x20);
	if (W25Nxx.ID >= W25N256)
		W25Nxx_Spi((SectorAddr & 0xFF000000) >> 24);
	W25Nxx_Spi((SectorAddr & 0xFF0000) >> 16);
	W25Nxx_Spi((SectorAddr & 0xFF00) >> 8);
	W25Nxx_Spi(SectorAddr & 0xFF);
	_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
	;
	W25Nxx_WaitForWriteEnd();
#if (_W25NXX_DEBUG == 1)
	printf("W25Nxx EraseSector done after %d ms\r\n", Get_Time_Stamp() - StartTime);
#endif
	W25Nxx_Delay(1);
	W25Nxx.Lock = 0;
}
*/

	//###################################################################################################################
	void W25Nxx_EraseBlock(uint32_t BlockAddr)
	{
#if (_W25NXX_DEBUG == 1)
		printf("W25Nxx EraseBlock %d Begin...\r\n", BlockAddr);
		W25Nxx_Delay(100);
		uint32_t StartTime = Get_Time_Stamp();
#endif
		W25Nxx_WaitForWriteEnd();
		BlockAddr = BlockAddr * W25Nxx.PageSize * 64;
		W25Nxx_WriteEnable();
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(CMD_BLOCK_ERASE);
		W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		W25Nxx_Spi((BlockAddr & 0xFF00) >> 8);
		W25Nxx_Spi(BlockAddr & 0xFF);
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		W25Nxx_WaitForWriteEnd();
#if (_W25NXX_DEBUG == 1)
		printf("W25Nxx EraseBlock done after %d ms\r\n", Get_Time_Stamp() - StartTime);
		W25Nxx_Delay(100);
#endif
		W25Nxx_Delay(1);
		W25Nxx.Lock = 0;
	}
	//###################################################################################################################
	uint32_t W25Nxx_PageToSector(uint32_t PageAddress)
	{
		return ((PageAddress * W25Nxx.PageSize) / W25Nxx.SectorSize);
	}
	//###################################################################################################################
	uint32_t W25Nxx_PageToBlock(uint32_t PageAddress)
	{
		return ((PageAddress * W25Nxx.PageSize) / W25Nxx.BlockSize);
	}
	//###################################################################################################################
	uint32_t W25Nxx_SectorToBlock(uint32_t SectorAddress)
	{
		return ((SectorAddress * W25Nxx.SectorSize) / W25Nxx.BlockSize);
	}
	//###################################################################################################################
	uint32_t W25Nxx_SectorToPage(uint32_t SectorAddress)
	{
		return (SectorAddress * W25Nxx.SectorSize) / W25Nxx.PageSize;
	}
	//###################################################################################################################
	uint32_t W25Nxx_BlockToPage(uint32_t BlockAddress)
	{
		return (BlockAddress * W25Nxx.BlockSize) / W25Nxx.PageSize;
	}
	//###################################################################################################################
	bool W25Nxx_IsEmptyPage(uint32_t Page, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize)
	{
		if (((NumByteToCheck_up_to_PageSize + OffsetInByte) > W25Nxx.PageSize) || (NumByteToCheck_up_to_PageSize == 0))
			NumByteToCheck_up_to_PageSize = W25Nxx.PageSize - OffsetInByte;
		uint8_t pBuffer[BUF_SIZE] = {0xFF};
		uint16_t Page_Address = Page * W25Nxx.PageSize;
		//Send page data to tmp buffer
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(CMD_PAGE_DATA_READ);
		W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		W25Nxx_Spi((Page_Address & 0xFF00) >> 8);
		W25Nxx_Spi(Page_Address & 0xFF);
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		W25Nxx_WaitForWriteEnd();
		W25Nxx_Delay(50);
		//read tmp buffer byte to byte
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(CMD_FAST_READ);
		W25Nxx_Spi((OffsetInByte & 0xFF00) >> 8);
		W25Nxx_Spi(OffsetInByte & 0xFF);
		for (unsigned int i = 0; i < NumByteToCheck_up_to_PageSize; i++)
		{
			pBuffer[i] = W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		}
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;

		for (unsigned int x = 0; x < BUF_SIZE; x++)
		{
			if (pBuffer[x] != 0xFF)
				return false;
		}
#if (_W25NXX_DEBUG == 1)
		printf("W25Nxx CheckPage is Empty in %d ms\r\n", Get_Time_Stamp() - StartTime);
		W25Nxx_Delay(100);
#endif
		W25Nxx.Lock = 0;
		return true;
	}
	//###################################################################################################################
	/*
bool W25Nxx_IsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize)
{
	while (W25Nxx.Lock == 1)
		W25Nxx_Delay(1);
	W25Nxx.Lock = 1;
	if ((NumByteToCheck_up_to_SectorSize > W25Nxx.SectorSize) || (NumByteToCheck_up_to_SectorSize == 0))
		NumByteToCheck_up_to_SectorSize = W25Nxx.SectorSize;
#if (_W25NXX_DEBUG == 1)
	printf("W25Nxx CheckSector:%d, Offset:%d, Bytes:%d begin...\r\n", Sector_Address, OffsetInByte, NumByteToCheck_up_to_SectorSize);
	W25Nxx_Delay(100);
	uint32_t StartTime = Get_Time_Stamp();
#endif
	uint8_t pBuffer[BUF_SIZE];
	uint32_t WorkAddress;
	uint32_t i;
	for (i = OffsetInByte; i < W25Nxx.SectorSize; i += BUF_SIZE)
	{
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		WorkAddress = (i + Sector_Address * W25Nxx.SectorSize);
		W25Nxx_Spi(0x0B);
		if (W25Nxx.ID >= W25N256)
			W25Nxx_Spi((WorkAddress & 0xFF000000) >> 24);
		W25Nxx_Spi((WorkAddress & 0xFF0000) >> 16);
		W25Nxx_Spi((WorkAddress & 0xFF00) >> 8);
		W25Nxx_Spi(WorkAddress & 0xFF);
		W25Nxx_Spi(0);
		HAL_SPI_Receive(&_W25NXX_SPI, pBuffer, BUF_SIZE, 100);
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		;
		for (uint8_t x = 0; x < BUF_SIZE; x++)
		{
			if (pBuffer[x] != 0xFF)
				goto NOT_EMPTY;
		}
	}
	if ((W25Nxx.SectorSize + OffsetInByte) % BUF_SIZE != 0)
	{
		i -= BUF_SIZE;
		for (; i < W25Nxx.SectorSize; i++)
		{
			_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
			WorkAddress = (i + Sector_Address * W25Nxx.SectorSize);
			W25Nxx_Spi(0x0B);
			if (W25Nxx.ID >= W25N256)
				W25Nxx_Spi((WorkAddress & 0xFF000000) >> 24);
			W25Nxx_Spi((WorkAddress & 0xFF0000) >> 16);
			W25Nxx_Spi((WorkAddress & 0xFF00) >> 8);
			W25Nxx_Spi(WorkAddress & 0xFF);
			W25Nxx_Spi(0);
			HAL_SPI_Receive(&_W25NXX_SPI, pBuffer, 1, 100);
			_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
			;
			if (pBuffer[0] != 0xFF)
				goto NOT_EMPTY;
		}
	}
#if (_W25NXX_DEBUG == 1)
	printf("W25Nxx CheckSector is Empty in %d ms\r\n", Get_Time_Stamp() - StartTime);
	W25Nxx_Delay(100);
#endif
	W25Nxx.Lock = 0;
	return true;
NOT_EMPTY:
#if (_W25NXX_DEBUG == 1)
	printf("W25Nxx CheckSector is Not Empty in %d ms\r\n", Get_Time_Stamp() - StartTime);
	W25Nxx_Delay(100);
#endif
	W25Nxx.Lock = 0;
	return false;
}
*/
	//###################################################################################################################
	/*
bool W25Nxx_IsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize)
{
	while (W25Nxx.Lock == 1)
		W25Nxx_Delay(1);
	W25Nxx.Lock = 1;
	if ((NumByteToCheck_up_to_BlockSize > W25Nxx.BlockSize) || (NumByteToCheck_up_to_BlockSize == 0))
		NumByteToCheck_up_to_BlockSize = W25Nxx.BlockSize;
#if (_W25NXX_DEBUG == 1)
	printf("W25Nxx CheckBlock:%d, Offset:%d, Bytes:%d begin...\r\n", Block_Address, OffsetInByte, NumByteToCheck_up_to_BlockSize);
	W25Nxx_Delay(100);
	uint32_t StartTime = Get_Time_Stamp();
#endif
	uint8_t pBuffer[BUF_SIZE];
	uint32_t WorkAddress;
	uint32_t i;
	for (i = OffsetInByte; i < W25Nxx.BlockSize; i += BUF_SIZE)
	{
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		WorkAddress = (i + Block_Address * W25Nxx.BlockSize);
		W25Nxx_Spi(0x0B);
		if (W25Nxx.ID >= W25N256)
			W25Nxx_Spi((WorkAddress & 0xFF000000) >> 24);
		W25Nxx_Spi((WorkAddress & 0xFF0000) >> 16);
		W25Nxx_Spi((WorkAddress & 0xFF00) >> 8);
		W25Nxx_Spi(WorkAddress & 0xFF);
		W25Nxx_Spi(0);
		HAL_SPI_Receive(&_W25NXX_SPI, pBuffer, BUF_SIZE, 100);
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		;
		for (uint8_t x = 0; x < BUF_SIZE; x++)
		{
			if (pBuffer[x] != 0xFF)
				goto NOT_EMPTY;
		}
	}
	if ((W25Nxx.BlockSize + OffsetInByte) % BUF_SIZE != 0)
	{
		i -= BUF_SIZE;
		for (; i < W25Nxx.BlockSize; i++)
		{
			_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
			WorkAddress = (i + Block_Address * W25Nxx.BlockSize);
			W25Nxx_Spi(0x0B);
			if (W25Nxx.ID >= W25N256)
				W25Nxx_Spi((WorkAddress & 0xFF000000) >> 24);
			W25Nxx_Spi((WorkAddress & 0xFF0000) >> 16);
			W25Nxx_Spi((WorkAddress & 0xFF00) >> 8);
			W25Nxx_Spi(WorkAddress & 0xFF);
			W25Nxx_Spi(0);
			HAL_SPI_Receive(&_W25NXX_SPI, pBuffer, 1, 100);
			_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
			;
			if (pBuffer[0] != 0xFF)
				goto NOT_EMPTY;
		}
	}
#if (_W25NXX_DEBUG == 1)
	printf("W25Nxx CheckBlock is Empty in %d ms\r\n", Get_Time_Stamp() - StartTime);
	W25Nxx_Delay(100);
#endif
	W25Nxx.Lock = 0;
	return true;
NOT_EMPTY:
#if (_W25NXX_DEBUG == 1)
	printf("W25Nxx CheckBlock is Not Empty in %d ms\r\n", Get_Time_Stamp() - StartTime);
	W25Nxx_Delay(100);
#endif
	W25Nxx.Lock = 0;
	return false;
}
*/
	//###################################################################################################################
	void W25Nxx_WriteByte(uint8_t pBuffer, uint32_t WriteAddr_inBytes)
	{
#if (_W25NXX_DEBUG == 1)
		uint32_t StartTime = Get_Time_Stamp();
		printf("W25Nxx WriteByte 0x%02X at address %d begin...", pBuffer, WriteAddr_inBytes);
#endif
		W25Nxx_WaitForWriteEnd();
		W25Nxx_WriteEnable();
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(CMD_DATA_LOAD);
		W25Nxx_Spi((WriteAddr_inBytes & 0xFF00) >> 8);
		W25Nxx_Spi(WriteAddr_inBytes & 0xFF);
		W25Nxx_Spi(pBuffer);
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		;
		W25Nxx_WaitForWriteEnd();
#if (_W25NXX_DEBUG == 1)
		printf("W25Nxx WriteByte done after %d ms\r\n", Get_Time_Stamp() - StartTime);
#endif
		W25Nxx.Lock = 0;
	}
	//###################################################################################################################
	void W25Nxx_WritePage(uint8_t * pBuffer,uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize)
	{
		if (((NumByteToWrite_up_to_PageSize + OffsetInByte) > W25Nxx.PageSize) || (NumByteToWrite_up_to_PageSize == 0))
			NumByteToWrite_up_to_PageSize = W25Nxx.PageSize - OffsetInByte;
		if ((OffsetInByte + NumByteToWrite_up_to_PageSize) > W25Nxx.PageSize)
			NumByteToWrite_up_to_PageSize = W25Nxx.PageSize - OffsetInByte;

		W25Nxx_WaitForWriteEnd();
		W25Nxx_WriteEnable();
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(CMD_DATA_LOAD);
		W25Nxx_Spi((OffsetInByte & 0xFF00) >> 8);
		W25Nxx_Spi(OffsetInByte & 0xFF);
		for (int i = 0; i < NumByteToWrite_up_to_PageSize; i++)
		{
			W25Nxx_Spi(pBuffer[i]);
		}
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		W25Nxx_WaitForWriteEnd();

		W25Nxx_Delay(1);
		W25Nxx.Lock = 0;
	}
//###################################################################################################################	
	void W25Nxx_Execute(uint32_t Page)
	{
		W25Nxx_WaitForWriteEnd();
		Page *= W25Nxx.PageSize;
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(CMD_PROGRAM_EXEC);
		W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		W25Nxx_Spi(Page >> 8);
		W25Nxx_Spi(Page);
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		;
		W25Nxx_WaitForWriteEnd();
#if (_W25NXX_DEBUG == 1)
		printf("W25Nxx WriteByte done after %d ms\r\n", Get_Time_Stamp() - StartTime);
#endif
		W25Nxx.Lock = 0;
	}
	//###################################################################################################################
	/*
void W25Nxx_WriteSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize)
{
	if ((NumByteToWrite_up_to_SectorSize > W25Nxx.SectorSize) || (NumByteToWrite_up_to_SectorSize == 0))
		NumByteToWrite_up_to_SectorSize = W25Nxx.SectorSize;
#if (_W25NXX_DEBUG == 1)
	printf("+++W25Nxx WriteSector:%d, Offset:%d ,Write %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToWrite_up_to_SectorSize);
	W25Nxx_Delay(100);
#endif
	if (OffsetInByte >= W25Nxx.SectorSize)
	{
#if (_W25NXX_DEBUG == 1)
		printf("---W25Nxx WriteSector Faild!\r\n");
		W25Nxx_Delay(100);
#endif
		return;
	}
	uint32_t StartPage;
	int32_t BytesToWrite;
	uint32_t LocalOffset;
	if ((OffsetInByte + NumByteToWrite_up_to_SectorSize) > W25Nxx.SectorSize)
		BytesToWrite = W25Nxx.SectorSize - OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_SectorSize;
	StartPage = W25Nxx_SectorToPage(Sector_Address) + (OffsetInByte / W25Nxx.PageSize);
	LocalOffset = OffsetInByte % W25Nxx.PageSize;
	do
	{
		W25Nxx_WritePage(pBuffer, StartPage, LocalOffset, BytesToWrite);
		StartPage++;
		BytesToWrite -= W25Nxx.PageSize - LocalOffset;
		pBuffer += W25Nxx.PageSize - LocalOffset;
		LocalOffset = 0;
	} while (BytesToWrite > 0);
#if (_W25NXX_DEBUG == 1)
	printf("---W25Nxx WriteSector Done\r\n");
	W25Nxx_Delay(100);
#endif
}
*/
	//###################################################################################################################
	void W25Nxx_WriteBlock(uint8_t * pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize)
	{
		if ((NumByteToWrite_up_to_BlockSize > W25Nxx.BlockSize) || (NumByteToWrite_up_to_BlockSize == 0))
			NumByteToWrite_up_to_BlockSize = W25Nxx.BlockSize;

		if (OffsetInByte >= W25Nxx.BlockSize)
		{

			return;
		}
		uint32_t StartPage;
		int32_t BytesToWrite;
		uint32_t LocalOffset;
		if ((OffsetInByte + NumByteToWrite_up_to_BlockSize) > W25Nxx.BlockSize)
			BytesToWrite = W25Nxx.BlockSize - OffsetInByte;
		else
			BytesToWrite = NumByteToWrite_up_to_BlockSize;
		StartPage = W25Nxx_BlockToPage(Block_Address) + (OffsetInByte / W25Nxx.PageSize);
		LocalOffset = OffsetInByte % W25Nxx.PageSize;
		do
		{
			W25Nxx_WritePage(pBuffer, LocalOffset, BytesToWrite);
			StartPage++;
			BytesToWrite -= W25Nxx.PageSize - LocalOffset;
			pBuffer += W25Nxx.PageSize - LocalOffset;
			LocalOffset = 0;
		} while (BytesToWrite > 0);
	}
	//###################################################################################################################
	void W25Nxx_ReadByte(uint8_t * pBuffer, uint32_t Bytes_Address)
	{
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(0x0B);
		W25Nxx_Spi((Bytes_Address & 0xFF00) >> 8);
		W25Nxx_Spi(Bytes_Address & 0xFF);
		W25Nxx_Spi(0);
		*pBuffer = W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		;
		W25Nxx.Lock = 0;
	}
	//###################################################################################################################
	void W25Nxx_ReadBytes(uint8_t * pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
	{
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(CMD_FAST_READ);
		W25Nxx_Spi((ReadAddr & 0xFF00) >> 8);
		W25Nxx_Spi(ReadAddr & 0xFF);
		W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		for (int i = 0; i < NumByteToRead; i++)
		{
			pBuffer[i] = W25Nxx_Spi(0x00);
		}

		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		;

		W25Nxx_Delay(1);
		W25Nxx.Lock = 0;
	}
	//###################################################################################################################
	void W25Nxx_ReadPage(uint8_t * pBuffer, uint32_t Page, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize)
	{

		if ((NumByteToRead_up_to_PageSize > W25Nxx.PageSize) || (NumByteToRead_up_to_PageSize == 0))
			NumByteToRead_up_to_PageSize = W25Nxx.PageSize;
		if ((OffsetInByte + NumByteToRead_up_to_PageSize) > W25Nxx.PageSize)
			NumByteToRead_up_to_PageSize = W25Nxx.PageSize - OffsetInByte;

		uint16_t Page_Address = Page * W25Nxx.PageSize;
		//send page data to buff
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(CMD_PAGE_DATA_READ);
		W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		W25Nxx_Spi((Page_Address & 0xFF00) >> 8);
		W25Nxx_Spi(Page_Address & 0xFF);
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;
		W25Nxx_WaitForWriteEnd();
		W25Nxx_Delay(50);
		//read buff byte to byte
		_W25NXX_CS_GPIO->BRR = _W25NXX_CS_PIN;
		W25Nxx_Spi(CMD_FAST_READ);
		W25Nxx_Spi((OffsetInByte & 0xFF00) >> 8);
		W25Nxx_Spi(OffsetInByte & 0xFF);
		W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		for (unsigned int i = 0; i < NumByteToRead_up_to_PageSize; i++)
		{
			pBuffer[i] = W25Nxx_Spi(W25NXX_DUMMY_BYTE);
		}
		_W25NXX_CS_GPIO->BSRR = _W25NXX_CS_PIN;

		W25Nxx_Delay(1);
		W25Nxx.Lock = 0;
	}
	//###################################################################################################################
	void W25Nxx_ReadSector(uint8_t * pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize)
	{
		if ((NumByteToRead_up_to_SectorSize > W25Nxx.SectorSize) || (NumByteToRead_up_to_SectorSize == 0))
			NumByteToRead_up_to_SectorSize = W25Nxx.SectorSize;
#if (_W25NXX_DEBUG == 1)
		printf("+++W25Nxx ReadSector:%d, Offset:%d ,Read %d Bytes, begin...\r\n", Sector_Address, OffsetInByte, NumByteToRead_up_to_SectorSize);
		W25Nxx_Delay(100);
#endif
		if (OffsetInByte >= W25Nxx.SectorSize)
		{
#if (_W25NXX_DEBUG == 1)
			printf("---W25Nxx ReadSector Faild!\r\n");
			W25Nxx_Delay(100);
#endif
			return;
		}
		uint32_t StartPage;
		int32_t BytesToRead;
		uint32_t LocalOffset;
		if ((OffsetInByte + NumByteToRead_up_to_SectorSize) > W25Nxx.SectorSize)
			BytesToRead = W25Nxx.SectorSize - OffsetInByte;
		else
			BytesToRead = NumByteToRead_up_to_SectorSize;
		StartPage = W25Nxx_SectorToPage(Sector_Address) + (OffsetInByte / W25Nxx.PageSize);
		LocalOffset = OffsetInByte % W25Nxx.PageSize;
		do
		{
			W25Nxx_ReadPage(pBuffer, StartPage, LocalOffset, BytesToRead);
			StartPage++;
			BytesToRead -= W25Nxx.PageSize - LocalOffset;
			pBuffer += W25Nxx.PageSize - LocalOffset;
			LocalOffset = 0;
		} while (BytesToRead > 0);
	}
	//###################################################################################################################
	void W25Nxx_ReadBlock(uint8_t * pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize)
	{
		if ((NumByteToRead_up_to_BlockSize > W25Nxx.BlockSize) || (NumByteToRead_up_to_BlockSize == 0))
			NumByteToRead_up_to_BlockSize = W25Nxx.BlockSize;

		if (OffsetInByte >= W25Nxx.BlockSize)
		{

			return;
		}
		uint32_t StartPage;
		int32_t BytesToRead;
		uint32_t LocalOffset;
		if ((OffsetInByte + NumByteToRead_up_to_BlockSize) > W25Nxx.BlockSize)
			BytesToRead = W25Nxx.BlockSize - OffsetInByte;
		else
			BytesToRead = NumByteToRead_up_to_BlockSize;
		StartPage = W25Nxx_BlockToPage(Block_Address) + (OffsetInByte / W25Nxx.PageSize);
		LocalOffset = OffsetInByte % W25Nxx.PageSize;
		do
		{
			W25Nxx_ReadPage(pBuffer, StartPage, LocalOffset, BytesToRead);
			StartPage++;
			BytesToRead -= W25Nxx.PageSize - LocalOffset;
			pBuffer += W25Nxx.PageSize - LocalOffset;
			LocalOffset = 0;
		} while (BytesToRead > 0);
	}
	//###################################################################################################################
