
#include "eeprom_demo.h"
#include "24cxx.h"



extern I2C_HandleTypeDef hi2c1;

// AT24C02���Ժ���
void AT24C02_Test(void)
{
    uint8_t testAddress = 0x00;  // ������ʼ��ַ
    uint8_t writeData[16] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                            0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};
    uint8_t readData[16] = {0};
    uint8_t singleByteWrite = 0xA5;
    uint8_t singleByteRead = 0;
    HAL_StatusTypeDef status;
    
    printf("\r\n===== AT24C02 EEPROM Test =====\r\n");
    
    // 1. ����豸�Ƿ���Ӧ
    printf("1. Checking device presence...");
    status = AT24C02_IsReady(&hi2c1, AT24C02_ADDRESS);
    if(status != HAL_OK)
    {
        printf(" FAILED! (Device not responding)\r\n");
        return;
    }
    printf(" OK\r\n");
    
    // 2. ���Ե��ֽڶ�д
    printf("2. Testing single byte R/W...");
    status = AT24C02_WriteByte(&hi2c1, AT24C02_ADDRESS, testAddress, singleByteWrite);
    if(status != HAL_OK)
    {
        printf(" WRITE FAILED!\r\n");
        return;
    }
    
    // �ȴ�д�����
    while(AT24C02_IsReady(&hi2c1, AT24C02_ADDRESS) != HAL_OK);
    
    status = AT24C02_ReadByte(&hi2c1, AT24C02_ADDRESS, testAddress, &singleByteRead);
    if(status != HAL_OK)
    {
        printf(" READ FAILED!\r\n");
        return;
    }
    
    if(singleByteRead != singleByteWrite)
    {
        printf(" VERIFY FAILED! (Wrote 0x%02X, Read 0x%02X)\r\n", singleByteWrite, singleByteRead);
        return;
    }
    printf(" OK (Wrote 0x%02X, Read 0x%02X)\r\n", singleByteWrite, singleByteRead);
    
    // 3. ���Զ��ֽ�������д(����ҳ)
    printf("3. Testing multi-byte R/W (same page)...");
    status = AT24C02_WritePage(&hi2c1, AT24C02_ADDRESS, testAddress, writeData, 8);
    if(status != HAL_OK)
    {
        printf(" WRITE FAILED!\r\n");
        return;
    }
    
    // �ȴ�д�����
    while(AT24C02_IsReady(&hi2c1, AT24C02_ADDRESS) != HAL_OK);
    
    status = AT24C02_ReadBuffer(&hi2c1, AT24C02_ADDRESS, testAddress, readData, 8);
    if(status != HAL_OK)
    {
        printf(" READ FAILED!\r\n");
        return;
    }
    
    if(memcmp(writeData, readData, 8) != 0)
    {
        printf(" VERIFY FAILED!\r\n");
        printf(" Written: ");
        for(int i = 0; i < 8; i++) printf("%02X ", writeData[i]);
        printf("\r\n Read:    ");
        for(int i = 0; i < 8; i++) printf("%02X ", readData[i]);
        printf("\r\n");
        return;
    }
    printf(" OK\r\n");
    
    // 4. ���Կ�ҳд��(���Ե�ַ��Ϊ�ӽ�ҳ�߽�)
    printf("4. Testing cross-page write...");
    testAddress = 0x07;  // ҳ�߽�ǰһ���ֽ�(ҳ��С8�ֽ�)
    status = AT24C02_WritePage(&hi2c1, AT24C02_ADDRESS, testAddress, writeData, 8);
    if(status == HAL_OK)
    {
        printf(" TEST FAILED! (Should not allow cross-page write)\r\n");
        return;
    }
    
    // ��ȷ�Ŀ�ҳд�뷽ʽ - ������д��
    status = AT24C02_WritePage(&hi2c1, AT24C02_ADDRESS, testAddress, writeData, 1); // д��ҳ�߽����һ���ֽ�
    if(status != HAL_OK)
    {
        printf(" WRITE FAILED!\r\n");
        return;
    }
    
    // �ȴ�д�����
    while(AT24C02_IsReady(&hi2c1, AT24C02_ADDRESS) != HAL_OK);
    
    status = AT24C02_WritePage(&hi2c1, AT24C02_ADDRESS, testAddress+1, writeData+1, 7); // д����һҳ��ǰ7���ֽ�
    if(status != HAL_OK)
    {
        printf(" WRITE FAILED!\r\n");
        return;
    }
    
    // �ȴ�д�����
    while(AT24C02_IsReady(&hi2c1, AT24C02_ADDRESS) != HAL_OK);
    
    status = AT24C02_ReadBuffer(&hi2c1, AT24C02_ADDRESS, testAddress, readData, 8);
    if(status != HAL_OK)
    {
        printf(" READ FAILED!\r\n");
        return;
    }
    
    if(memcmp(writeData, readData, 8) != 0)
    {
        printf(" VERIFY FAILED!\r\n");
        printf(" Written: ");
        for(int i = 0; i < 8; i++) printf("%02X ", writeData[i]);
        printf("\r\n Read:    ");
        for(int i = 0; i < 8; i++) printf("%02X ", readData[i]);
        printf("\r\n");
        return;
    }
    printf(" OK\r\n");
    
    // 5. ��������оƬ��д
    printf("5. Testing full chip R/W (this may take a while)...\r\n");
    uint8_t pattern = 0xAA;
    for(uint16_t addr = 0; addr < 256; addr++)
    {
        status = AT24C02_WriteByte(&hi2c1, AT24C02_ADDRESS, addr, pattern);
        if(status != HAL_OK)
        {
            printf("Write failed at address 0x%02X\r\n", addr);
            return;
        }
        
        // �ȴ�д�����
        while(AT24C02_IsReady(&hi2c1, AT24C02_ADDRESS) != HAL_OK);
        
        pattern = ~pattern; // ����д��0xAA��0x55
    }
    
    // ��֤����д�������
    pattern = 0xAA;
    for(uint16_t addr = 0; addr < 256; addr++)
    {
        uint8_t readByte;
        status = AT24C02_ReadByte(&hi2c1, AT24C02_ADDRESS, addr, &readByte);
        if(status != HAL_OK)
        {
            printf("Read failed at address 0x%02X\r\n", addr);
            return;
        }
        
        if(readByte != pattern)
        {
            printf("Verify failed at address 0x%02X (Wrote 0x%02X, Read 0x%02X)\r\n", 
                       addr, pattern, readByte);
            return;
        }
        
        pattern = ~pattern;
        
        // ÿ64�ֽڴ�ӡһ�����ȵ�
        if((addr % 64) == 0) printf(".");
    }
    printf(" OK\r\n");
    
    printf("===== AT24C02 Test PASSED! =====\r\n");
}



