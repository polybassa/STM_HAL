// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <cstdint>
#include <tuple>
#include <cstring>
#include "os_Task.h"
#include "for_each_tuple.h"
#include "CRC.h"

#ifdef UNITTEST
int ut_tuple(void);
int ut_tupleCopy(void);
int ut_makeDto(void);
#endif

namespace com
{
template<typename ... types>
class DataTransferObject
{
    std::tuple<types& ...> mTransferTuple;

    static constexpr size_t DATASIZE = pack_size<types ...> ::value;

    typedef struct __attribute__((packed)) {
        uint32_t timestamp;
        uint8_t data [DATASIZE];
        uint8_t crc;
    } DataTransferStruct;

    DataTransferStruct mTransferData;

public:
    DataTransferObject(types& ... tuple) :
        mTransferTuple(tuple ...) {}

    DataTransferObject(const DataTransferObject&) = delete;
    DataTransferObject(DataTransferObject&&) = default;
    DataTransferObject& operator=(const DataTransferObject&) = delete;
    DataTransferObject& operator=(DataTransferObject&&) = delete;

    void updateTuple(void);
    void prepareForTx(void);
    bool isValid(void);

    inline uint8_t* data(void)
    {
        return reinterpret_cast<uint8_t*>(&mTransferData);
    }

    constexpr inline size_t length(void) const
    {
        return sizeof(mTransferData);
    }

#ifdef UNITTEST
    friend int ::ut_tuple(void);
    friend int ::ut_tupleCopy(void);
    friend int ::ut_makeDto(void);
#endif
};

template<typename ... _Elements>
constexpr com::DataTransferObject<typename std::remove_pointer_t<std::decay_t<_Elements> > ...>
make_dto(_Elements&& ... __args)
{
    typedef com::DataTransferObject<typename std::remove_pointer_t<std::decay_t<_Elements> > ...> __result_type;
    return __result_type(std::forward<_Elements>(__args) ...);
}
}

template<typename ... types>
void com::DataTransferObject<types ...>::prepareForTx(void)
{
    mTransferData.timestamp = os::Task::getTickCount();
    uint8_t* ptr = mTransferData.data;

    for_each(mTransferTuple, [&ptr](const auto& x){
        std::memcpy(ptr, &x, sizeof(x));
        ptr += sizeof(x);
    });
    const hal::Crc& crcUnit = hal::Factory<hal::Crc>::get<hal::Crc::SYSTEM_CRC>();
    mTransferData.crc = crcUnit.getCrc(this->data(), this->length() - sizeof(mTransferData.crc));
}

template<typename ... types>
bool com::DataTransferObject<types ...>::isValid(void)
{
    const hal::Crc& crcUnit = hal::Factory<hal::Crc>::get<hal::Crc::SYSTEM_CRC>();
    const bool crcValid = (0x00 == (crcUnit.getCrc(this->data(), this->length())));
    return crcValid;
}

template<typename ... types>
void com::DataTransferObject<types ...>::updateTuple(void)
{
    uint8_t const* ptr = mTransferData.data;
    os::ThisTask::enterCriticalSection();
    for_each(mTransferTuple, [&ptr](auto& x){
        std::memcpy(&x, ptr, sizeof(x));
        ptr += sizeof(x);
    });
    os::ThisTask::exitCriticalSection();
}
