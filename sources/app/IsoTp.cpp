#include <cstring>
#include "os_Task.h"
#include "Can.h"
#include "IsoTp.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using app::ISOTP;

ISOTP::ISOTP(const hal::Can& interface, const uint32_t sid, const uint32_t did) : mInterface(interface), mSid(sid),
    mDid(did)
{
    Trace(ZONE_INFO, "Constructor\r\n");
}

size_t ISOTP::send_Message(const std::string_view message, const std::chrono::milliseconds timeout)
{
    if (message.size() > ISOTP::MAX_ISOTP_PAYLOAD) {
        Trace(ZONE_INFO, "Message in ISOTP has a max length of 4095 Bytes.\r\n");
        return 0;
    }
    if (message.size() <= MAX_SINGLE_FRAME_PAYLOAD) {
        return ISOTP::send_SF(message);
    }
    ISOTP::send_FF(message);
    if (!ISOTP::receive_FC(timeout)) {
        return 0;
    }
    return ISOTP::send_CF(message);
}

size_t ISOTP::receive_Message(char* buffer, const size_t length, const std::chrono::milliseconds timeout)
{
    std::memset(&mCanRxMsg, 0, sizeof(mCanRxMsg));
    std::memset(buffer, 0, sizeof(length));
    uint32_t startTime = os::Task::getTickCount();

    while (os::Task::getTickCount() < (startTime + timeout.count())) {
        if (!mInterface.receive(mCanRxMsg)) {
            continue;
        }
        if (mCanRxMsg.StdId != mDid) {
            return 0;
        }
        const auto receivedFrame = std::string_view(reinterpret_cast<const char*>(mCanRxMsg.Data),
                                                    sizeof(mCanRxMsg.Data));

        switch (receivedFrame[0] >> 4) {
        case FrameTypes::SINGLE_FRAME:
            return receive_SF(receivedFrame, buffer, length);

        case FrameTypes::FIRST_FRAME:
            return receive_FF(receivedFrame, buffer, length) > 0 ? receive_CF(buffer, length, timeout) : 0;

        default:
            Trace(ZONE_ERROR, "Not received a single or first frame because byte 0 is = %d\r\n", receivedFrame[0]);
            return 0;
        }
    }
    Trace(ZONE_INFO, "Receiving Frame failed probably due timeout.\r\n");
    return 0;
}

size_t ISOTP::send_SF(const std::string_view message)
{
    if (mSid > 0x7ff) {
        Trace(ZONE_WARNING, "Extended IDs not supported yet.\r\n");
        return 0;
    }
    std::memset(&mCanTxMsg, 0, sizeof(mCanTxMsg));
    mCanTxMsg.StdId = mSid;
    mCanTxMsg.DLC = message.size() + 1;
    mCanTxMsg.Data[0] = (FrameTypes::SINGLE_FRAME << 4) + (message.size() & 0x0f);
    std::memcpy(mCanTxMsg.Data + 1, message.data(), message.size());
    mInterface.send(mCanTxMsg);
    return message.size();
}

void ISOTP::send_FF(const std::string_view message)
{
    if (mSid > 0x7ff) {
        Trace(ZONE_WARNING, "Extended IDs not supported yet.\r\n");
        return;
    }
    std::memset(&mCanTxMsg, 0, sizeof(mCanTxMsg));
    mCanTxMsg.StdId = mSid;
    mCanTxMsg.DLC = 8;
    mCanTxMsg.Data[0] = (FrameTypes::FIRST_FRAME << 4) + ((message.size() & 0xf00) >> 8);
    mCanTxMsg.Data[1] = message.size() & 0xff;
    std::memcpy(mCanTxMsg.Data + 2, message.data(), 6);
    mInterface.send(mCanTxMsg);
}

void ISOTP::send_FC(const size_t length, const FlowControlStatus& status)
{
    std::memset(&mCanTxMsg, 0, sizeof(mCanTxMsg));
    mCanTxMsg.StdId = 0x7ff & mSid;
    mCanTxMsg.DLC = 3;
    mCanTxMsg.Data[0] = (FrameTypes::FLOW_CONTROL << 4) + (status & 0x0f);
    mCanTxMsg.Data[1] = length / 7;
    mCanTxMsg.Data[2] = std::chrono::milliseconds(1).count();
    mInterface.send(mCanTxMsg);
}

size_t ISOTP::send_CF(const std::string_view message)
{
    uint8_t sequenzNumber = 1;
    size_t index = 6;

    std::memset(&mCanTxMsg, 0, sizeof(mCanTxMsg));
    mCanTxMsg.StdId = 0x7ff & mSid;
    while (message.size() > index) {
        if (mSeperationTime.count()) {
            os::ThisTask::sleep(mSeperationTime);
        }
        const auto framelength = std::min(MAX_SINGLE_FRAME_PAYLOAD, static_cast<uint8_t>(message.size() - index));
        mCanTxMsg.DLC = framelength + 1;
        std::memset(mCanTxMsg.Data, 0, sizeof(mCanTxMsg.Data));
        mCanTxMsg.Data[0] = (FrameTypes::CONSECUTIVE_FRAME << 4) + sequenzNumber;
        sequenzNumber = (sequenzNumber + 1) & 0x0F;
        std::memcpy(mCanTxMsg.Data + 1, message.data() + index, framelength);
        mInterface.send(mCanTxMsg);
        index += framelength;
    }
    return index;
}

size_t ISOTP::receive_SF(const std::string_view message, char* buffer, const size_t length)
{
    mRxMsgLength = message[0];
    if (length < mRxMsgLength) {
        Trace(ZONE_INFO, "Message is to long for buffer.\r\n");
        return 0;
    }
    if (mRxMsgLength > MAX_SINGLE_FRAME_PAYLOAD) {
        Trace(ZONE_INFO, "Message is to long for single Frame\r\n");
        return 0;
    }
    std::memcpy(buffer, message.data() + 1, mRxMsgLength);
    return mRxMsgLength;
}

size_t ISOTP::receive_FF(const std::string_view message, char* buffer, const size_t length)
{
    mRxMsgLength = ((message[0] & 0x0f) << 8) | (message[1]);

    if (mRxMsgLength <= MAX_SINGLE_FRAME_PAYLOAD) {
        Trace(ZONE_INFO, "Message is to short for a first frame.\r\n");
        return 0;
    }
    if (mRxMsgLength > length) {
        Trace(ZONE_INFO, "Message is to long for buffer Overflow.\r\n");
        send_FC(mRxMsgLength, FlowControlStatus::FS_Overflow);
        return 0;
    }
    std::memcpy(buffer, (message.data() + 2), message.size() - 2);
    send_FC(mRxMsgLength, FlowControlStatus::FS_Clear_To_Send);
    return message.size() - 2;
}

size_t ISOTP::receive_CF(char* buffer, const size_t length, const std::chrono::milliseconds timeout)
{
    size_t sequenzNumberOfConsecutiveFrame = 1;
    size_t index = 6;
    uint32_t startTime = os::Task::getTickCount();

    while (mRxMsgLength > index) {
        if (os::Task::getTickCount() > (startTime + timeout.count())) {
            Trace(ZONE_INFO, "Receiving Consecutive Frame failed probably due timeout.\r\n");
            return 0;
        }

        std::memset(&mCanRxMsg, 0, sizeof(CanRxMsg));
        if (!mInterface.receive(mCanRxMsg)) {
            continue;
        }
        if (mCanRxMsg.Data[0] >> 4 != FrameTypes::CONSECUTIVE_FRAME) {
            return 0;
        }
        if (sequenzNumberOfConsecutiveFrame != (mCanRxMsg.Data[0] & 0x0f)) {
            return 0;
        }

        std::memcpy(buffer + index, (mCanRxMsg.Data + 1), mCanRxMsg.DLC - 1);
        sequenzNumberOfConsecutiveFrame = (sequenzNumberOfConsecutiveFrame + 1) & 0x0f;
        index += mCanRxMsg.DLC - 1;
        startTime = os::Task::getTickCount();
    }
    return index;
}

bool ISOTP::receive_FC(const std::chrono::milliseconds timeout)
{
    if (mDid > 0x7ff) {
        Trace(ZONE_WARNING, "Extended IDs not supported yet.\r\n");
        return 0;
    }
    const uint32_t startTime = os::Task::getTickCount();

    while (os::Task::getTickCount() < (startTime + timeout.count())) {
        if (!mInterface.receive(mCanRxMsg)) {
            continue;
        }
        if ((mCanRxMsg.StdId != mDid) && (mCanRxMsg.DLC != 3) &&
            (mCanRxMsg.Data[0] != (FrameTypes::FLOW_CONTROL << 4 & 0xf0)))
        {
            Trace(ZONE_INFO, "False flow control status received or has a different did or frame size is to long.\r\n");
            return false;
        }
        mBlockSize = mCanRxMsg.Data[1];
        mSeperationTime = std::chrono::milliseconds(mCanRxMsg.Data[2]);
        return true;
    }
    Trace(ZONE_INFO, "Receiving Flow Control failed probably due timeout.\r\n");
    return false;
}
