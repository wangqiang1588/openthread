/*
 *  Copyright (c) 2016, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 *   This file includes definitions for managing MeshCoP Datasets.
 *
 */

#ifndef MESHCOP_DATASET_MANAGER_HPP_
#define MESHCOP_DATASET_MANAGER_HPP_

#include <openthread/types.h>

#include "coap/coap.hpp"
#include "common/locator.hpp"
#include "common/timer.hpp"
#include "meshcop/dataset.hpp"
#include "net/udp6.hpp"
#include "thread/mle.hpp"
#include "thread/network_data_leader.hpp"

namespace ot {

class ThreadNetif;

namespace MeshCoP {

class DatasetManager: public ThreadNetifLocator
{
public:
    Dataset &GetLocal(void) { return mLocal; }
    Dataset &GetNetwork(void) { return mNetwork; }

    otError ApplyConfiguration(void);

protected:
    enum
    {
        kFlagLocalUpdated   = 1 << 0,
        kFlagNetworkUpdated = 1 << 1,
    };

    DatasetManager(ThreadNetif &aThreadNetif, const Tlv::Type aType, const char *aUriSet, const char *aUriGet,
                   Timer::Handler aTimerHander);

    otError Clear(uint8_t &aFlags, bool aOnlyClearNetwork);

    otError Set(const Dataset &aDataset);

    otError Set(const Timestamp &aTimestamp, const Message &aMessage, uint16_t aOffset, uint8_t aLength,
                uint8_t &aFlags);

    void Get(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

    void HandleNetworkUpdate(uint8_t &aFlags);
    void HandleTimer(void);

    Dataset mLocal;
    Dataset mNetwork;

private:
    static void HandleUdpReceive(void *aContext, otMessage *aMessage, const otMessageInfo *aMessageInfo);
    void HandleUdpReceive(Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

    otError Register(void);
    void SendGetResponse(const Coap::Header &aRequestHeader, const Ip6::MessageInfo &aMessageInfo,
                         uint8_t *aTlvs, uint8_t aLength);
    Timer mTimer;

    const char *mUriSet;
    const char *mUriGet;

#if OPENTHREAD_FTD
public:
    otError SendSetRequest(const otOperationalDataset &aDataset, const uint8_t *aTlvs, uint8_t aLength);
    otError SendGetRequest(const uint8_t *aTlvTypes, uint8_t aLength, const otIp6Address *aAddress);

protected:
    otError Set(const otOperationalDataset &aDataset, uint8_t &aFlags);
    otError Set(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

private:
    void SendSetResponse(const Coap::Header &aRequestHeader, const Ip6::MessageInfo &aMessageInfo, StateTlv::State aState);
#endif
};

class ActiveDatasetBase: public DatasetManager
{
public:
    ActiveDatasetBase(ThreadNetif &aThreadNetif);

    otError Restore(void);

    otError Clear(bool aOnlyClearNetwork);

#if OPENTHREAD_FTD
    otError Set(const otOperationalDataset &aDataset);
#endif

    otError Set(const Dataset &aDataset);

    otError Set(const Timestamp &aTimestamp, const Message &aMessage, uint16_t aOffset, uint8_t aLength);

private:
    static void HandleGet(void *aContext, otCoapHeader *aHeader, otMessage *aMessage,
                          const otMessageInfo *aMessageInfo);
    void HandleGet(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

    static void HandleTimer(Timer &aTimer);
    void HandleTimer(void) { DatasetManager::HandleTimer(); }

    Coap::Resource mResourceGet;
};

class PendingDatasetBase: public DatasetManager
{
public:
    PendingDatasetBase(ThreadNetif &aThreadNetif);

    otError Restore(void);

    otError Clear(bool aOnlyClearNetwork);

#if OPENTHREAD_FTD
    otError Set(const otOperationalDataset &aDataset);
#endif

    otError Set(const Dataset &aDataset);

    otError Set(const Timestamp &aTimestamp, const Message &aMessage, uint16_t aOffset, uint8_t aLength);

    void UpdateDelayTimer(void);

protected:
    static void HandleDelayTimer(Timer &aTimer);
    void HandleDelayTimer(void);

    void ResetDelayTimer(uint8_t aFlags);
    void UpdateDelayTimer(Dataset &aDataset, uint32_t &aStartTime);

    void HandleNetworkUpdate(uint8_t &aFlags);

    Timer mDelayTimer;
    uint32_t mLocalTime;
    uint32_t mNetworkTime;

private:
    static void HandleGet(void *aContext, otCoapHeader *aHeader, otMessage *aMessage,
                          const otMessageInfo *aMessageInfo);
    void HandleGet(Coap::Header &aHeader, Message &aMessage, const Ip6::MessageInfo &aMessageInfo);

    static void HandleTimer(Timer &aTimer);
    void HandleTimer(void) { DatasetManager::HandleTimer(); }

    Coap::Resource mResourceGet;
};

}  // namespace MeshCoP
}  // namespace ot

#if OPENTHREAD_MTD
#include "dataset_manager_mtd.hpp"
#elif OPENTHREAD_FTD
#include "dataset_manager_ftd.hpp"
#else
#error Must define OPENTHREAD_MTD or OPENTHREAD_FTD
#endif

#endif  // MESHCOP_DATASET_MANAGER_HPP_
