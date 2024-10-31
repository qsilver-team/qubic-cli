// #include <cstdint>
// #include <cstring>
#include "structs.h"
// #include "walletUtils.h"
// #include "keyUtils.h"
// #include "assetUtil.h"
#include "connection.h"
#include "logger.h"
#include "nodeUtils.h"
// #include "K12AndKeyUtil.h"
#include "vliquid.h"

#define EXAM_PUBLIC_CONTRACT_INDEX 9
#define EXAM_PUBLIC_INPUT_TYPE 3

void callExamPublicFunction(const char* nodeIp, const int nodePort, uint64_t inputValue, uint64_t& outputValue) {
    auto qc = make_qc(nodeIp, nodePort);

    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        ExamPublic_input input;
    } packet;

    // Set up packet
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    packet.rcf.inputSize = sizeof(ExamPublic_input);
    packet.rcf.inputType = EXAM_PUBLIC_INPUT_TYPE;  // Replace with the actual input type ID for ExamPublic
    packet.rcf.contractIndex = EXAM_PUBLIC_CONTRACT_INDEX;  // Replace with actual contract index for ExamPublic

    // Initialize input value
    packet.input.inputValue = inputValue;

    // Send the request
    qc->sendData((uint8_t*)&packet, packet.header.size());

    // // Prepare to receive data
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;
    bool success = false;

    // Parse response
    while (ptr < recvByte) {
        auto header = (RequestResponseHeader*)(data + ptr);
        if (header->type() == RespondContractFunction::type()) {
            if (recvByte - ptr - sizeof(RequestResponseHeader) >= sizeof(ExamPublic_output)) {
                auto response = (ExamPublic_output*)(data + ptr + sizeof(RequestResponseHeader));
                outputValue = response->outputValue;
                LOG("Result: %d\n", outputValue);
                success = true;
                break;
            }
        }
        ptr += header->size();
    }
    const char* boolStr = success ? "true" : "false";
    if (!success) {
        LOG("Failed");
    }
}

void qxExamPublic(const char* nodeIp, int nodePort,
                        const char* inputValue)
{
    auto qc = make_qc(nodeIp, nodePort);
    uint64_t outputValue;
    uint64_t value = std::stoull(inputValue);
    callExamPublicFunction(nodeIp, nodePort, value, outputValue);
}
