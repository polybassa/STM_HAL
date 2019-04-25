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

#define PN532_PREAMBLE                      (0x00)
#define PN532_STARTCODE1                    (0x00)
#define PN532_STARTCODE2                    (0xFF)
#define PN532_POSTAMBLE                     (0x00)

#define PN532_HOSTTOPN532                   (0xD4)
#define PN532_PN532TOHOST                   (0xD5)

// PN532 Commands
#define PN532_COMMAND_DIAGNOSE              (0x00)
#define PN532_COMMAND_GETFIRMWAREVERSION    (0x02)
#define PN532_COMMAND_GETGENERALSTATUS      (0x04)
#define PN532_COMMAND_READREGISTER          (0x06)
#define PN532_COMMAND_WRITEREGISTER         (0x08)
#define PN532_COMMAND_READGPIO              (0x0C)
#define PN532_COMMAND_WRITEGPIO             (0x0E)
#define PN532_COMMAND_SETSERIALBAUDRATE     (0x10)
#define PN532_COMMAND_SETPARAMETERS         (0x12)
#define PN532_COMMAND_SAMCONFIGURATION      (0x14)
#define PN532_COMMAND_POWERDOWN             (0x16)
#define PN532_COMMAND_RFCONFIGURATION       (0x32)
#define PN532_COMMAND_RFREGULATIONTEST      (0x58)
#define PN532_COMMAND_INJUMPFORDEP          (0x56)
#define PN532_COMMAND_INJUMPFORPSL          (0x46)
#define PN532_COMMAND_INLISTPASSIVETARGET   (0x4A)
#define PN532_COMMAND_INATR                 (0x50)
#define PN532_COMMAND_INPSL                 (0x4E)
#define PN532_COMMAND_INDATAEXCHANGE        (0x40)
#define PN532_COMMAND_INCOMMUNICATETHRU     (0x42)
#define PN532_COMMAND_INDESELECT            (0x44)
#define PN532_COMMAND_INRELEASE             (0x52)
#define PN532_COMMAND_INSELECT              (0x54)
#define PN532_COMMAND_INAUTOPOLL            (0x60)
#define PN532_COMMAND_TGINITASTARGET        (0x8C)
#define PN532_COMMAND_TGSETGENERALBYTES     (0x92)
#define PN532_COMMAND_TGGETDATA             (0x86)
#define PN532_COMMAND_TGSETDATA             (0x8E)
#define PN532_COMMAND_TGSETMETADATA         (0x94)
#define PN532_COMMAND_TGGETINITIATORCOMMAND (0x88)
#define PN532_COMMAND_TGRESPONSETOINITIATOR (0x90)
#define PN532_COMMAND_TGGETTARGETSTATUS     (0x8A)

#define PN532_RESPONSE_INDATAEXCHANGE       (0x41)
#define PN532_RESPONSE_INLISTPASSIVETARGET  (0x4B)

#define PN532_WAKEUP                        (0x55)

#define PN532_SPI_STATREAD                  (0x02)
#define PN532_SPI_DATAWRITE                 (0x01)
#define PN532_SPI_DATAREAD                  (0x03)
#define PN532_SPI_READY                     (0x01)

const os::TaskEndless gpioTest("Gpio_Test", 1024, os::Task::Priority::MEDIUM, [](const bool&){
                               Trace(ZONE_INFO, "HI Markus\r\n");

                               constexpr const hal::Gpio& out = hal::Factory<hal::Gpio>::get<hal::Gpio::LED>();
                               constexpr const hal::Gpio& spi_cs = hal::Factory<hal::Gpio>::get<hal::Gpio::SPI1_NSS>();

                               constexpr const hal::Spi& spi = hal::Factory<hal::Spi>::get<hal::Spi::PN532SPI>();
                               spi_cs = true;

                               os::ThisTask::sleep(std::chrono::milliseconds(1000));
                               while (true) {
                                   const uint8_t cmdlen = 2;
                                   const uint8_t ncmdlen = ~cmdlen;
                                   const uint8_t cmd = PN532_COMMAND_GETFIRMWAREVERSION;
                                   uint8_t checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
                                   checksum += PN532_HOSTTOPN532;
                                   checksum += cmd;
                                   uint8_t nchecksum = (uint8_t) ~checksum;

                                   uint8_t data[] = {PN532_SPI_DATAWRITE,
                                                     PN532_PREAMBLE, PN532_PREAMBLE, PN532_STARTCODE2,
                                                     cmdlen, ncmdlen + 1,
                                                     PN532_HOSTTOPN532,
                                                     cmd,
                                                     nchecksum,
                                                     PN532_POSTAMBLE };

                                   spi_cs = false;
                                   spi.receive(); // clear RXNE
                                   spi.send(data, sizeof(data));
                                   while (!spi.isReadyToReceive()) {
                                       ;
                                   }
                                   spi_cs = true;

                                   uint8_t status = 0;

                                   for (int i = 0; i < 5; i++) {
                                       spi_cs = false;
                                       uint8_t ready[] = {0x02};
                                       spi.send(ready, sizeof(ready));
                                       auto ret = spi.receive(&status, 1);
                                       spi_cs = true;
                                       Trace(ZONE_INFO, "ret %02d stat %02d\r\n", ret, status);
                                       if (status == 1) { break;}
                                       os::ThisTask::sleep(std::chrono::milliseconds(1));
                                   }
                                   if (status != 1) {continue;}

                                   spi_cs = false;
                                   std::array<uint8_t, 7> read = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                                   spi.send(read.data(), 1);
                                   auto ret = spi.receive(read.data() + 1, 6);
                                   spi_cs = true;
                                   std::array<uint8_t, 14> readPrint;
                                   hexlify(readPrint, read);
                                   Trace(ZONE_INFO, "ret %02d stat %s\r\n", ret, readPrint.data());
                                   os::ThisTask::sleep(std::chrono::milliseconds(1000));

//                                   auto x = getChar();
//                                   if (x) {
//                                       spi_cs = false;
//                                       spi.send(x);
//                                       os::ThisTask::sleep(std::chrono::milliseconds(1));
//                                       spi_cs = true;
//                                       out = true;
//                                       os::ThisTask::sleep(std::chrono::milliseconds(100));
//                                       out = false;
//                                   }
                               }
    });
