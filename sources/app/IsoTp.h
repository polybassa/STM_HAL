#include <chrono>
#include <string_view>
#include "Can.h"

namespace app
{
class ISOTP
{
    static constexpr const uint16_t MAX_ISOTP_PAYLOAD = 4095;
    static constexpr const uint8_t MAX_SINGLE_FRAME_PAYLOAD = 7;

    enum FrameTypes {
        SINGLE_FRAME = 0x00,
        FIRST_FRAME = 0x01,
        CONSECUTIVE_FRAME = 0x02,
        FLOW_CONTROL = 0x03
    };

    enum FlowControlStatus {
        FS_Clear_To_Send = 0x00,
        FS_Wait = 0x01,
        FS_Overflow = 0x02
    };

    const hal::Can& mInterface;
    CanTxMsg mCanTxMsg;
    CanRxMsg mCanRxMsg;
    uint32_t mSid;
    uint32_t mDid;
    std::chrono::milliseconds mSeperationTime;
    size_t mBlockSize;
    size_t mRxMsgLength;

    size_t send_SF(const std::string_view message);
    void send_FF(const std::string_view message);
    size_t send_CF(const std::string_view message);
    void send_FC(const size_t length, const FlowControlStatus& status);
    bool receive_FC(const std::chrono::milliseconds timeout);
    size_t receive_SF(const std::string_view message, char* buffer, const size_t length);
    size_t receive_FF(const std::string_view message, char* buffer, const size_t length);
    size_t receive_CF(char* buffer, const size_t length, const std::chrono::milliseconds timeout);

public:
    ISOTP(const hal::Can& interface, const uint32_t sid, const uint32_t did);
    size_t send_Message(const std::string_view message, const std::chrono::milliseconds timeout);
    size_t receive_Message(char* buffer, const size_t length, const std::chrono::milliseconds timeout);
};
}
