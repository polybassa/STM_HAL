/* Copyright (C) 2015  Nils Weiss
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "TestGpio.h"
#include "Gpio.h"
#include "trace.h"
#include "Usb.h"
#include "Tim.h"
#include "Spi.h"
#include <array>
#include "binascii.h"
#include "PN_532.h"
#include <cstring>

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

char getChar(void)
{
    char ret = 0;
    constexpr const auto& Usb = hal::Factory<hal::Usb>::get();

    if (Usb.isConfigured()) {
        auto len = Usb.receive((uint8_t*)&ret, sizeof(ret), std::chrono::milliseconds(1000));
        if (len != sizeof(ret)) {
            Trace(ZONE_INFO, "Receive Failed\r\n");
            return 0;
        }
    }
    Trace(ZONE_INFO, "Receive Char %c bytesLeft = %d\r\n", ret, 1);

    return ret;
}

const os::TaskEndless gpioTest("Gpio_Test", 2048, os::Task::Priority::MEDIUM, [](const bool&){
                               Trace(ZONE_INFO, "Started\r\n");

                               constexpr const hal::Gpio& out = hal::Factory<hal::Gpio>::get<hal::Gpio::LED>();
                               constexpr const hal::Gpio& spi_cs = hal::Factory<hal::Gpio>::get<hal::Gpio::SPI1_NSS>();

                               constexpr const hal::Spi& spi = hal::Factory<hal::Spi>::get<hal::Spi::PN532SPI>();

                               Adafruit_PN532 nfc(spi_cs, spi);

                               os::ThisTask::sleep(std::chrono::milliseconds(3000));
                               nfc.begin();

                               uint32_t versiondata = nfc.getFirmwareVersion();
                               if (!versiondata) {
                                   Trace(ZONE_INFO, "Didn't find PN53x board");
                                   while (1) {
                                       ; // halt
                                   }
                               }
                               // Got ok data, print it out!
                               Trace(ZONE_INFO, "Found chip PN5%x\r\n", (versiondata >> 24) & 0xFF);
                               Trace(ZONE_INFO,
                                     "Firmware ver. %d.%d\r\n",
                                     (versiondata >> 16) & 0xFF,
                                     (versiondata >> 8) & 0xFF);

                               // configure board to read RFID tags
                               nfc.SAMConfig();

                               Trace(ZONE_INFO, "Waiting for an ISO14443A Card ...\r\n");

                               while (true) {
                                   os::ThisTask::sleep(std::chrono::milliseconds(100));
                                   uint8_t success;                          // Flag to check if there was an error with the PN532
                                   uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
                                   uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
                                   uint8_t currentblock;                     // Counter to keep track of which block we're on
                                   bool authenticated = false;               // Flag to indicate if the sector is authenticated
                                   uint8_t data[16];                         // Array to store block data during reads

                                   // Keyb on NDEF and Mifare Classic should be the same
                                   uint8_t keyuniversal[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

                                   // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
                                   // 'uid' will be populated with the UID, and uidLength will indicate
                                   // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
                                   success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

                                   if (success) {
                                       // Display some basic information about the card
                                       Trace(ZONE_INFO,
                                             "Found an ISO14443A card  UID Length: %d bytes  UID Value: ",
                                             uidLength);
                                       nfc.PrintHex(uid, uidLength);
                                       Trace(ZONE_INFO, "\r\n");

                                       if (uidLength == 4) {
                                           // We probably have a Mifare Classic card ...
                                           Trace(ZONE_INFO, "Seems to be a Mifare Classic card (4 byte UID)\r\n");

                                           // Now we try to go through all 16 sectors (each having 4 blocks)
                                           // authenticating each sector, and then dumping the blocks
                                           for (currentblock = 0; currentblock < 64; currentblock++) {
                                               // Check if this is a new block so that we can reauthenticate
                                               if (nfc.mifareclassic_IsFirstBlock(currentblock)) {
                                                   authenticated = false;
                                               }

                                               // If the sector hasn't been authenticated, do so first
                                               if (!authenticated) {
                                                   // Starting of a new sector ... try to to authenticate
                                                   Trace(ZONE_INFO,
                                                         "------------------------Sector %d -------------------------\r\n",
                                                         currentblock / 4);

                                                   if (currentblock == 0) {
                                                       // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
                                                       // or 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 for NDEF formatted cards using key a,
                                                       // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
                                                       success = nfc.mifareclassic_AuthenticateBlock(uid,
                                                                                                     uidLength,
                                                                                                     currentblock,
                                                                                                     1,
                                                                                                     keyuniversal);
                                                   } else {
                                                       // This will be 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF for Mifare Classic (non-NDEF!)
                                                       // or 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 for NDEF formatted cards using key a,
                                                       // but keyb should be the same for both (0xFF 0xFF 0xFF 0xFF 0xFF 0xFF)
                                                       success = nfc.mifareclassic_AuthenticateBlock(uid,
                                                                                                     uidLength,
                                                                                                     currentblock,
                                                                                                     1,
                                                                                                     keyuniversal);
                                                   }
                                                   if (success) {
                                                       authenticated = true;
                                                   } else {
                                                       Trace(ZONE_INFO, "Authentication error\r\n");
                                                   }
                                               }
                                               // If we're still not authenticated just skip the block
                                               if (!authenticated) {
                                                   Trace(ZONE_INFO, "Block %d unable to authenticate\r\n",
                                                         currentblock);
                                               } else {
                                                   // Authenticated ... we should be able to read the block now
                                                   // Dump the data into the 'data' array
                                                   success = nfc.mifareclassic_ReadDataBlock(currentblock, data);
                                                   if (success) {
                                                       // Read successful
                                                       Trace(ZONE_INFO, "Block %d \r\n", currentblock);
                                                       // Dump the raw data
                                                       std::array<uint8_t, 128> dataBuffer;
                                                       std::array<uint8_t, 256> printBuffer;

                                                       const size_t dataLength = 16;
                                                       std::memcpy(dataBuffer.data(), data, dataLength);

                                                       hexlify(printBuffer, dataBuffer);
                                                       printBuffer[dataLength] = 0;
                                                       Trace(ZONE_INFO, "%s\r\n", printBuffer.data());
                                                   } else {
                                                       // Oops ... something happened
                                                       Trace(ZONE_INFO,
                                                             "Block %d unable to read this block\r\n",
                                                             currentblock);
                                                   }
                                               }
                                           }
                                       } else {
                                           Trace(ZONE_INFO,
                                                 "Ooops ... this doesn't seem to be a Mifare Classic card!\r\n");
                                       }
                                   }
                                   // Wait a bit before trying again
                                   Trace(ZONE_INFO, "\n\nSend a character to run the mem dumper again!\r\n");
                                   os::ThisTask::sleep(std::chrono::milliseconds(10000));
                               }
    });
