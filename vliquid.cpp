#include <cstdint>
#include <cstring>
#include "structs.h"
#include "walletUtils.h"
#include "keyUtils.h"
#include "assetUtil.h"
#include "connection.h"
#include "logger.h"
#include "nodeUtils.h"
#include "K12AndKeyUtil.h"
#include "vliquid.h"
#include "vliquidStruct.h"

// Define constants for VLIQUID contract
#define VLIQUID_CONTRACT_INDEX 9
#define VLIQUID_CONTRACTID "3" // temp

// VLIQUID FUNCTIONS
#define VLIQUID_MICRO_TOKEN_ALLOWANCE 1
#define VLIQUID_BALANCE_OF_MICRO_TOKEN 2
#define VLIQUID_EXAM_PUBLIC 3

// VLIQUID PROCEDURES
#define VLIQUID_APPROVE_MICRO_TOKEN 1
#define VLIQUID_TRANSFER_MICRO_TOKEN 2
#define VLIQUID_TRANSFER_FROM_MICRO_TOKEN 3
#define VLIQUID_CONVERT_TO_MICRO_TOKEN 4
#define VLIQUID_CONVERT_TO_EXPENSIVE_TOKEN 5
#define VLIQUID_CREATE_LIQUID 6
#define VLIQUID_ADD_LIQUID 7
#define VLIQUID_REMOVE_LIQUID 8
#define VLIQUID_SWAP_TO_QU 9
#define VLIQUID_SWAP_FROM_QU 10
#define VLIQUID_SWAP_TO_QWALLET 11
#define VLIQUID_SWAP_FROM_QWALLET 12
#define VLIQUID_SWAP_QU_TO_QWALLET 13
#define VLIQUID_SWAP_QWALLET_TO_QU 14
#define VLIQUID_SINGLE_SWAP 15
#define VLIQUID_CROSS_SWAP 16

void vliquidBalanceOfMicroToken(const char* nodeIp, int nodePort, const char* assetName, const char* issuer, const char* owner) {
    auto qc = make_qc(nodeIp, nodePort);
    
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        BalanceOfMicroToken_input bmti;
    } packet;
    
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    
    packet.rcf.inputSize = sizeof(BalanceOfMicroToken_input);
    packet.rcf.inputType = VLIQUID_BALANCE_OF_MICRO_TOKEN;
    packet.rcf.contractIndex = VLIQUID_CONTRACT_INDEX;

    uint8_t issuerPublicKey[32] = {0};
    getPublicKeyFromIdentity(issuer, issuerPublicKey);
    memcpy(packet.bmti.issuer, issuerPublicKey, 32);

    memcpy(&packet.bmti.assetName, assetName, 8);
    memcpy(packet.bmti.owner, owner, 32);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;

    ConvertToMicroToken_output result;
    memset(&result, 0, sizeof(result));
    while (ptr < recvByte)
    {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RespondContractFunction::type()){
            if (recvByte - ptr - sizeof(RequestResponseHeader) >= sizeof(ConvertToMicroToken_output)){
                auto output = (ConvertToMicroToken_output*)(data + ptr + sizeof(RequestResponseHeader));
                result = *output;
            } else {
                LOG("Error: Insufficient data for ConvertToMicroToken_output\n");
                LOG("Received bytes: %d, Expected: %d\n", recvByte - ptr - sizeof(RequestResponseHeader), sizeof(ConvertToMicroToken_output));
            }
        } else {
            LOG("Unexpected header type: %d\n", header->type());
        }
        ptr+= header->size();
    }

    LOG("Balance of micro token: %lld\n", result.microTokenAmount);
}

void vliquidMicroTokenAllowance(const char* nodeIp, int nodePort, const char* assetName, const char* issuer, const char* recipient, const char* spender) {
}