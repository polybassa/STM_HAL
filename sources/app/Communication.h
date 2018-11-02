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

#ifndef SOURCES_PMD_APP_COMMUNICATION_H_
#define SOURCES_PMD_APP_COMMUNICATION_H_

#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "UsartWithDma.h"

namespace app
{
template<typename rxDto, typename txDto>
struct Communication final :
    private os::DeepSleepModule {
    enum class ErrorCode {
        CRC_ERROR = 0,
        OFFSET_ERROR,
        UPDATE_ERROR,
        NO_COMMUNICATION_ERROR,
        TX_ERROR
    };

    Communication(const hal::UsartWithDma& interface, rxDto&, txDto&,
                  std::function<void(ErrorCode)> errorCallback = nullptr);

    Communication(const Communication&) = delete;
    Communication(Communication&&) = default;
    Communication& operator=(const Communication&) = delete;
    Communication& operator=(Communication&&) = delete;

#ifdef UNITTEST
    void triggerRxTaskExecution(void) { this->RxTaskFunction(true); }
    void triggerTxTaskExecution(void) { this->TxTaskFunction(true); }
#endif

private:
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 1024;

    const hal::UsartWithDma& mInterface;
    rxDto& mRxDto;
    txDto& mTxDto;
    std::function<void(ErrorCode)> mErrorCallback;

    os::TaskInterruptable mTxTask;
    os::TaskInterruptable mRxTask;

    void TxTaskFunction(const bool&);
    void RxTaskFunction(const bool&);
};
}

template<typename rxDto, typename txDto>
app::Communication<rxDto, txDto>::Communication(const hal::UsartWithDma& interface, rxDto& rx_dto, txDto& tx_dto,
                                                std::function<void(ErrorCode)> errorCallback) :
    os::DeepSleepModule(),
        mInterface(interface),
    mRxDto(rx_dto),
    mTxDto(tx_dto),
    mErrorCallback(errorCallback),
    mTxTask("4ComTx",
            Communication::STACKSIZE,
            os::Task::Priority::VERY_HIGH,
            [this](const bool& join)
{
    TxTaskFunction(join);
}),
    mRxTask("5ComRx",
            Communication::STACKSIZE,
            os::Task::Priority::VERY_HIGH,
            [this](const bool& join)
{
    RxTaskFunction(join);
})
{}

template<typename rxDto, typename txDto>
void app::Communication<rxDto, txDto>::enterDeepSleep(void)
{
    //interface stop;
    mTxTask.join();
    mRxTask.join();
}

template<typename rxDto, typename txDto>
void app::Communication<rxDto, txDto>::exitDeepSleep(void)
{
    mRxTask.start();
    mTxTask.start();
}

template<typename rxDto, typename txDto>
void app::Communication<rxDto, txDto>::TxTaskFunction(const bool& join)
{
    do {
        mTxDto.prepareForTx();

        constexpr uint32_t ticksToWaitForTx = 30;
        const auto bytesTransmitted = mInterface.send(mTxDto.data(),
                                                      mTxDto.length(),
                                                      ticksToWaitForTx);

        if (bytesTransmitted != mTxDto.length()) {
            if (mErrorCallback) {
                mErrorCallback(ErrorCode::TX_ERROR);
            }
        }
        os::ThisTask::sleep(std::chrono::milliseconds(10));
    } while (!join);
}

template<typename rxDto, typename txDto>
void app::Communication<rxDto, txDto>::RxTaskFunction(const bool& join)
{
    mInterface.enableReceiveTimeout(10);

    do {
        os::ThisTask::sleep(std::chrono::milliseconds(9));

        constexpr uint32_t ticksToWaitForRx = 30;

        const auto bytesReceived = mInterface.receiveWithTimeout(mRxDto.data(),
                                                                 mRxDto.length(),
                                                                 ticksToWaitForRx);

        if (bytesReceived != mRxDto.length()) {
            if (mErrorCallback) {
                mErrorCallback(ErrorCode::NO_COMMUNICATION_ERROR);
            }
            continue;
        }

        if (!mRxDto.isValid()) {
            if (mErrorCallback) {
                mErrorCallback(ErrorCode::CRC_ERROR);
            }
            continue;
        }

        mRxDto.updateTuple();
    } while (!join);

    mInterface.disableReceiveTimeout();
}

#endif /* SOURCES_PMD_APP_COMMUNICATION_H_ */
