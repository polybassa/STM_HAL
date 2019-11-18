// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 * Modified 2019 by Henning Mende
 */

#pragma once

#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "UsartWithDma.h"
#include "Gpio.h"

namespace com
{
enum class ErrorCode : uint8_t {
    CRC_ERROR = 0,
    OFFSET_ERROR,
    UPDATE_ERROR,
    NO_COMMUNICATION_ERROR,
    TX_ERROR
};
}

namespace app
{
template<typename rxDto, typename txDto>
struct Communication final :
    private os::DeepSleepModule {
    Communication(const hal::UsartWithDma& interface, rxDto&, txDto&,
                  const std::chrono::milliseconds& transferPeriodMS,
                  std::function<void(com::ErrorCode)> errorCallback = nullptr);
    Communication(const hal::UsartWithDma& interface,
                  rxDto& rx, txDto& tx,
                  std::function<void(com::ErrorCode)> errorCallback = nullptr) :
        Communication(interface, rx, tx, std::chrono::milliseconds(10), errorCallback){}

    Communication(const Communication&) = delete;
    Communication(Communication&&) = default;
    Communication& operator=(const Communication&) = delete;
    Communication& operator=(Communication&&) = delete;

    inline bool isConnected(void) const
    {
        return mConnected;
    }

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
    const uint8_t mTransferPeriod;
    std::function<void(com::ErrorCode)> mErrorCallback;
    bool mConnected = false;

    os::TaskInterruptable mTxTask;
    os::TaskInterruptable mRxTask;

    void TxTaskFunction(const bool&);
    void RxTaskFunction(const bool&);
};
}

template<typename rxDto, typename txDto>
app::Communication<rxDto, txDto>::Communication(const hal::UsartWithDma& interface, rxDto& rx_dto, txDto& tx_dto,
                                                const std::chrono::milliseconds& transferPeriodMS,
                                                std::function<void(com::ErrorCode)> errorCallback) :
    os::DeepSleepModule(),
        mInterface(interface),
    mRxDto(rx_dto),
    mTxDto(tx_dto),
    mTransferPeriod(transferPeriodMS.count()),
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
                mErrorCallback(com::ErrorCode::TX_ERROR);
            }
        }
        os::ThisTask::sleep(std::chrono::milliseconds(mTransferPeriod));
    } while (!join);
}

template<typename rxDto, typename txDto>
void app::Communication<rxDto, txDto>::RxTaskFunction(const bool& join)
{
    mInterface.enableReceiveTimeout(10);
    const uint32_t ticksToWaitForRx = mTransferPeriod * 2;
    const std::chrono::milliseconds rxPeriod = std::chrono::milliseconds(mTransferPeriod - 1);

    do {
        const auto bytesReceived = mInterface.receiveWithTimeout(mRxDto.data(),
                                                                 mRxDto.length(),
                                                                 ticksToWaitForRx);

        if (bytesReceived != mRxDto.length()) {
            if (mErrorCallback) {
                mErrorCallback(com::ErrorCode::NO_COMMUNICATION_ERROR);
            }
            mConnected = false;
            continue;
        }

        if (!mRxDto.isValid()) {
            if (mErrorCallback) {
                mErrorCallback(com::ErrorCode::CRC_ERROR);
            }
            mConnected = false;
            // consume remaining bytes in hardware buffer
            auto bytesReceived = mInterface.receiveWithTimeout(mRxDto.data(),
                                                               mRxDto.length(),
                                                               mTransferPeriod / 2);
            continue;
        }

        mRxDto.updateTuple();

        // This is only reached if a connection is established.
        if (mErrorCallback) {
            mErrorCallback(com::ErrorCode::NO_COMMUNICATION_ERROR);
        }
        mConnected = true;

        os::ThisTask::sleep(rxPeriod);
    } while (!join);

    mInterface.disableReceiveTimeout();
}
