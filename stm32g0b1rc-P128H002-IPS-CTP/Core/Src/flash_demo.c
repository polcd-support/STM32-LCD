#include "flash_demo.h"
#include "w25qxx.h"
#include "stdio.h"
#include "stdint.h"
#include "sys.h"       // For timing functions (adjust based on your platform)
#include "delay.h"     // For delay functions

#define TEST_SECTOR        0x000000  // Sector address to test
#define TEST_PAGE          0x001000  // Page address to test
#define TEST_BLOCK         0x010000  // Block address to test
#define BUFFER_SIZE        256       // Test buffer size
#define SPEED_TEST_SIZE    4096      // Size for speed tests (4KB)
#define SECTOR_SIZE        4096      // 4KB sector size
#define BLOCK_SIZE         65536     // 64KB block size

const u8 TEXT_Buffer[]={"Explorer STM32F4 SPI TEST"};
#define SIZE sizeof(TEXT_Buffer)

void flash_demo(void){

	u16 flash_id;
    u8 status_reg;
    u8 write_buf[BUFFER_SIZE];
    u8 read_buf[BUFFER_SIZE];
    u32 start_time, end_time;
    float time_ms, speed_kbs;
    
    // Initialize the flash
    W25QXX_Init();
    
    // 1. Read Flash ID
    flash_id = W25QXX_ReadID();
    printf("\r\nFlash ID: 0x%04X\r\n", flash_id);
    
    if(flash_id != W25Q64 && flash_id != BY25Q64 && flash_id != NM25Q64) {
        printf("Error: Not a W25Q64 or compatible flash\r\n");
        return;
    }
    
    // 2. Read Status Register
    status_reg = W25QXX_ReadSR(1);
    printf("Status Register 1: 0x%02X\r\n", status_reg);
    
    // 3. Speed Test: Page Program (write)
    printf("\r\nTesting Page Program Speed...\r\n");
    
    // Prepare test data
    for(int i = 0; i < BUFFER_SIZE; i++) {
        write_buf[i] = i % 256;
    }
    
    // Erase sector first
    W25QXX_Write_Enable();
    W25QXX_Erase_Sector(TEST_SECTOR);
    W25QXX_Wait_Busy();
    
    // Time the write operation
    start_time = HAL_GetTick();  // Use your system timer function
    for(int i = 0; i < SPEED_TEST_SIZE/BUFFER_SIZE; i++) {
        W25QXX_Write_Enable();
        W25QXX_Write(write_buf, TEST_PAGE + (i * BUFFER_SIZE), BUFFER_SIZE);
        W25QXX_Wait_Busy();
    }
    end_time = HAL_GetTick();
    
    time_ms = (float)(end_time - start_time);
    speed_kbs = (SPEED_TEST_SIZE / time_ms) * 1000.0 / 1024.0;
    printf("Write Speed: %.2f KB/s (%.1f ms for %d bytes)\r\n", 
          speed_kbs, time_ms, SPEED_TEST_SIZE);
    
    // 4. Speed Test: Read
    printf("\r\nTesting Read Speed...\r\n");
    
    start_time = HAL_GetTick();
    for(int i = 0; i < SPEED_TEST_SIZE/BUFFER_SIZE; i++) {
        W25QXX_Read(read_buf, TEST_PAGE + (i * BUFFER_SIZE), BUFFER_SIZE);
    }
    end_time = HAL_GetTick();
    
    time_ms = (float)(end_time - start_time);
    speed_kbs = (SPEED_TEST_SIZE / time_ms) * 1000.0 / 1024.0;
    printf("Read Speed: %.2f KB/s (%.1f ms for %d bytes)\r\n", 
          speed_kbs, time_ms, SPEED_TEST_SIZE);
    
    // 5. Speed Test: Sector Erase (4KB)
    printf("\r\nTesting Sector Erase Speed...\r\n");
    
    start_time = HAL_GetTick();
    W25QXX_Write_Enable();
    W25QXX_Erase_Sector(TEST_SECTOR);
    W25QXX_Wait_Busy();
    end_time = HAL_GetTick();
    
    time_ms = (float)(end_time - start_time);
    printf("Sector Erase Time: %.1f ms (4KB)\r\n", time_ms);
    
    // 6. Speed Test: Block Erase (64KB)
    printf("\r\nTesting Block Erase Speed...\r\n");
    
    start_time = HAL_GetTick();
    W25QXX_Write_Enable();
    W25QXX_Erase_Block(TEST_BLOCK);
    W25QXX_Wait_Busy();
    end_time = HAL_GetTick();
    
    time_ms = (float)(end_time - start_time);
    printf("Block Erase Time: %.1f ms (64KB)\r\n", time_ms);
    
    // 7. Verify data integrity after speed tests
    printf("\r\nVerifying data integrity...\r\n");
    
    int errors = 0;
    for(int i = 0; i < SPEED_TEST_SIZE/BUFFER_SIZE; i++) {
        W25QXX_Read(read_buf, TEST_PAGE + (i * BUFFER_SIZE), BUFFER_SIZE);
        
        for(int j = 0; j < BUFFER_SIZE; j++) {
            if(read_buf[j] != (j % 256)) {
                errors++;
                if(errors < 5) {  // Print first few errors only
                    printf("Error at addr 0x%06X: wrote 0x%02X, read 0x%02X\r\n", 
                          TEST_PAGE + (i * BUFFER_SIZE) + j, j % 256, read_buf[j]);
                }
            }
        }
    }
    
    if(errors == 0) {
        printf("Data verification passed!\r\n");
    } else {
        printf("Data verification failed with %d errors\r\n", errors);
    }
    
    // 8. Test power down and wakeup timing
    printf("\r\nTesting power down/wakeup timing...\r\n");
    
    start_time = HAL_GetTick();
    W25QXX_PowerDown();
    end_time = HAL_GetTick();
    printf("Power down time: %lu ms\r\n", end_time - start_time);
    
    HAL_Delay(10);  // Short delay in power down mode
    
    start_time = HAL_GetTick();
    W25QXX_WAKEUP();
    end_time = HAL_GetTick();
    printf("Wakeup time: %lu ms\r\n", end_time - start_time);
    
    printf("\r\nW25Q64 Speed Test Complete!\r\n");

}



