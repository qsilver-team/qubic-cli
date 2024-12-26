#include <cstdint>
#include <cstring>
#include <sstream>
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

void vliquidMicroTokenAllowance(const char* nodeIp, int nodePort, 
                               const char* assetName, const char* issuer, 
                               const char* recipient, const char* spender) {
    auto qc = make_qc(nodeIp, nodePort);
    
    struct {
        RequestResponseHeader header;
        RequestContractFunction rcf;
        MicroTokenAllowance_input mtai;
    } packet;
    
    packet.header.setSize(sizeof(packet));
    packet.header.randomizeDejavu();
    packet.header.setType(RequestContractFunction::type());
    
    packet.rcf.inputSize = sizeof(MicroTokenAllowance_input);
    packet.rcf.inputType = VLIQUID_MICRO_TOKEN_ALLOWANCE;
    packet.rcf.contractIndex = VLIQUID_CONTRACT_INDEX;

    // Convert issuer identity to public key
    uint8_t issuerPublicKey[32] = {0};
    getPublicKeyFromIdentity(issuer, issuerPublicKey);
    memcpy(packet.mtai.issuer, issuerPublicKey, 32);

    // Copy asset name
    memcpy(&packet.mtai.assetName, assetName, 8);

    // Convert recipient identity to public key
    uint8_t recipientPublicKey[32] = {0};
    getPublicKeyFromIdentity(recipient, recipientPublicKey); 
    memcpy(packet.mtai.recipient, recipientPublicKey, 32);

    // Convert spender identity to public key
    uint8_t spenderPublicKey[32] = {0};
    getPublicKeyFromIdentity(spender, spenderPublicKey);
    memcpy(packet.mtai.spender, spenderPublicKey, 32);

    // Send request
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Receive response
    std::vector<uint8_t> buffer;
    qc->receiveDataAll(buffer);
    uint8_t* data = buffer.data();
    int recvByte = buffer.size();
    int ptr = 0;

    MicroTokenAllowance_output result;
    memset(&result, 0, sizeof(result));
    
    while (ptr < recvByte) {
        auto header = (RequestResponseHeader*)(data+ptr);
        if (header->type() == RespondContractFunction::type()) {
            if (recvByte - ptr - sizeof(RequestResponseHeader) >= sizeof(MicroTokenAllowance_output)) {
                auto output = (MicroTokenAllowance_output*)(data + ptr + sizeof(RequestResponseHeader));
                result = *output;
            } else {
                LOG("Error: Insufficient data for MicroTokenAllowance_output\n");
                LOG("Received bytes: %d, Expected: %d\n", 
                    recvByte - ptr - sizeof(RequestResponseHeader), 
                    sizeof(MicroTokenAllowance_output));
            }
        } else {
            LOG("Unexpected header type: %d\n", header->type());
        }
        ptr += header->size();
    }

    LOG("Micro token allowance: %lld\n", result.balance);
}

void vliquidApproveMicroToken(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* assetName,
                              const char* issuer,
                              const char* recipient,
                              uint64_t microTokenAmount,
                              uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        ApproveMicroToken_input amti;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_APPROVE_MICRO_TOKEN;
    packet.transaction.inputSize = sizeof(ApproveMicroToken_input);

    // Fill the input
    uint8_t issuerPublicKey[32] = {0};
    uint8_t recipientPublicKey[32] = {0};
    getPublicKeyFromIdentity(issuer, issuerPublicKey);
    getPublicKeyFromIdentity(recipient, recipientPublicKey);
    
    memcpy(packet.amti.issuer, issuerPublicKey, 32);
    memcpy(packet.amti.recipient, recipientPublicKey, 32);
    memcpy(&packet.amti.assetName, assetName, 8);
    packet.amti.microTokenAmount = microTokenAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(ApproveMicroToken_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(ApproveMicroToken_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(ApproveMicroToken_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.amti));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidConvertToMicroToken(const char* nodeIp, int nodePort,
                               const char* seed,
                               const char* assetName,
                               const char* issuer,
                               int64_t expensiveTokenAmount,
                               uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        ConvertToMicroToken_input cmti;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_CONVERT_TO_MICRO_TOKEN;
    packet.transaction.inputSize = sizeof(ConvertToMicroToken_input);

    // Fill the input
    uint8_t issuerPublicKey[32] = {0};
    getPublicKeyFromIdentity(issuer, issuerPublicKey);
    
    memcpy(packet.cmti.issuer, issuerPublicKey, 32);
    memcpy(&packet.cmti.assetName, assetName, 8);
    packet.cmti.expensiveTokenAmount = expensiveTokenAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(ConvertToMicroToken_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(ConvertToMicroToken_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(ConvertToMicroToken_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.cmti));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidConvertToExpensiveToken(const char* nodeIp, int nodePort,
                                   const char* seed,
                                   const char* assetName,
                                   const char* issuer,
                                   uint64_t microTokenAmount,
                                   uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        ConvertToExpensiveToken_input ceti;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_CONVERT_TO_EXPENSIVE_TOKEN;
    packet.transaction.inputSize = sizeof(ConvertToExpensiveToken_input);

    // Fill the input
    uint8_t issuerPublicKey[32] = {0};
    getPublicKeyFromIdentity(issuer, issuerPublicKey);
    
    memcpy(packet.ceti.issuer, issuerPublicKey, 32);
    memcpy(&packet.ceti.assetName, assetName, 8);
    packet.ceti.microTokenAmount = microTokenAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(ConvertToExpensiveToken_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(ConvertToExpensiveToken_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(ConvertToExpensiveToken_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.ceti));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidTransferMicroToken(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* assetName,
                              const char* issuer,
                              const char* recipient,
                              uint64_t microTokenAmount,
                              uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        TransferMicroToken_input tmti;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_TRANSFER_MICRO_TOKEN;
    packet.transaction.inputSize = sizeof(TransferMicroToken_input);

    // Fill the input
    uint8_t issuerPublicKey[32] = {0};
    uint8_t recipientPublicKey[32] = {0};
    getPublicKeyFromIdentity(issuer, issuerPublicKey);
    getPublicKeyFromIdentity(recipient, recipientPublicKey);
    
    memcpy(packet.tmti.issuer, issuerPublicKey, 32);
    memcpy(packet.tmti.recipient, recipientPublicKey, 32);
    memcpy(&packet.tmti.assetName, assetName, 8);
    packet.tmti.microTokenAmount = microTokenAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(TransferMicroToken_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(TransferMicroToken_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(TransferMicroToken_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.tmti));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidTransferFromMicroToken(const char* nodeIp, int nodePort,
                                  const char* seed,
                                  const char* assetName,
                                  const char* issuer,
                                  const char* spender,
                                  const char* recipient,
                                  uint64_t microTokenAmount,
                                  uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        TransferFromMicroToken_input tfmti;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_TRANSFER_FROM_MICRO_TOKEN;
    packet.transaction.inputSize = sizeof(TransferFromMicroToken_input);

    // Fill the input
    uint8_t issuerPublicKey[32] = {0};
    uint8_t spenderPublicKey[32] = {0};
    uint8_t recipientPublicKey[32] = {0};
    getPublicKeyFromIdentity(issuer, issuerPublicKey);
    getPublicKeyFromIdentity(spender, spenderPublicKey);
    getPublicKeyFromIdentity(recipient, recipientPublicKey);
    
    memcpy(packet.tfmti.issuer, issuerPublicKey, 32);
    memcpy(packet.tfmti.spender, spenderPublicKey, 32);
    memcpy(packet.tfmti.recipient, recipientPublicKey, 32);
    memcpy(&packet.tfmti.assetName, assetName, 8);
    packet.tfmti.microTokenAmount = microTokenAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(TransferFromMicroToken_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(TransferFromMicroToken_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(TransferFromMicroToken_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.tfmti));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidCreateLiquid(const char* nodeIp, int nodePort,
                        const char* seed,
                        const char* tokens,
                        uint8_t tokenLength,
                        uint64_t quShares,
                        uint8_t quWeight,
                        uint16_t initialLiquid,
                        uint8_t feeRate,
                        uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        CreateLiquid_input cli;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_CREATE_LIQUID;
    packet.transaction.inputSize = sizeof(CreateLiquid_input);

    // Parse tokens string and fill the input
    parseTokensString(tokens, tokenLength, packet.cli.tokens);
    packet.cli.tokenLength = tokenLength;
    packet.cli.quShares = quShares;
    packet.cli.quWeight = quWeight;
    packet.cli.initialLiquid = initialLiquid;
    packet.cli.feeRate = feeRate;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(CreateLiquid_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(CreateLiquid_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(CreateLiquid_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.cli));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

// Helper function to parse tokens string
void parseTokensString(const char* tokensStr, uint8_t tokenLength, Token* tokens) {
    std::string str(tokensStr);
    std::stringstream ss(str);
    std::string token;
    int idx = 0;
    
    while (std::getline(ss, token, ';') && idx < tokenLength) {
        std::stringstream tokenSS(token);
        std::string field;
        std::vector<std::string> fields;
        
        while (std::getline(tokenSS, field, ',')) {
            fields.push_back(field);
        }
        
        if (fields.size() == 5) {
            memcpy(&tokens[idx].tokenInfo.assetName, fields[0].c_str(), 8);
            getPublicKeyFromIdentity(fields[1].c_str(), tokens[idx].tokenInfo.issuer);
            tokens[idx].tokenInfo.isMicroToken = (fields[2] == "1");
            tokens[idx].balance = std::stoull(fields[3]);
            tokens[idx].weight = std::stoul(fields[4]);
            LOG("Token %d: %s, %s, %s, %llu, %d\n", idx, fields[0].c_str(), fields[1].c_str(), fields[2].c_str(), tokens[idx].balance, tokens[idx].weight);
            idx++;
        }
    }
}

void vliquidAddLiquid(const char* nodeIp, int nodePort,
                      const char* seed,
                      uint64_t tokenContribution,
                      uint64_t liquidId,
                      uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        AddLiquid_input ali;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_ADD_LIQUID;
    packet.transaction.inputSize = sizeof(AddLiquid_input);

    // Fill the input
    packet.ali.tokenContribution = tokenContribution;
    packet.ali.liquidId = liquidId;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(AddLiquid_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(AddLiquid_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(AddLiquid_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.ali));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidRemoveLiquid(const char* nodeIp, int nodePort,
                        const char* seed,
                        uint64_t tokenContribution,
                        uint64_t liquidId,
                        uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        RemoveLiquid_input rli;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_REMOVE_LIQUID;
    packet.transaction.inputSize = sizeof(RemoveLiquid_input);

    // Fill the input
    packet.rli.tokenContribution = tokenContribution;
    packet.rli.liquidId = liquidId;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(RemoveLiquid_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(RemoveLiquid_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(RemoveLiquid_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.rli));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidSwapToQU(const char* nodeIp, int nodePort,
                     const char* seed,
                     uint64_t liquidId,
                     const char* inputTokenInfo,
                     uint64_t inputAmount,
                     uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SwapToQU_input stqi;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_SWAP_TO_QU;
    packet.transaction.inputSize = sizeof(SwapToQU_input);

    // Parse input token info and fill the input
    std::string str(inputTokenInfo);
    std::stringstream ss(str);
    std::string token;
    std::vector<std::string> fields;
    
    while (std::getline(ss, token, ',')) {
        fields.push_back(token);
    }
    
    if (fields.size() == 2) {
        memcpy(&packet.stqi.inputTokenInfo.assetName, fields[0].c_str(), 8);
        getPublicKeyFromIdentity(fields[1].c_str(), packet.stqi.inputTokenInfo.issuer);
    }
    
    packet.stqi.liquidId = liquidId;
    packet.stqi.inputAmount = inputAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapToQU_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(SwapToQU_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapToQU_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.stqi));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidSwapFromQU(const char* nodeIp, int nodePort,
                       const char* seed,
                       uint64_t liquidId,
                       const char* outputTokenInfo,
                       uint64_t quAmount,
                       uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SwapFromQU_input sfqi;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_SWAP_FROM_QU;
    packet.transaction.inputSize = sizeof(SwapFromQU_input);

    // Parse output token info and fill the input
    std::string str(outputTokenInfo);
    std::stringstream ss(str);
    std::string token;
    std::vector<std::string> fields;
    
    while (std::getline(ss, token, ',')) {
        fields.push_back(token);
    }
    
    if (fields.size() == 2) {
        memcpy(&packet.sfqi.outputTokenInfo.assetName, fields[0].c_str(), 8);
        getPublicKeyFromIdentity(fields[1].c_str(), packet.sfqi.outputTokenInfo.issuer);
    }
    
    packet.sfqi.liquidId = liquidId;
    packet.sfqi.quAmount = quAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapFromQU_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(SwapFromQU_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapFromQU_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.sfqi));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidSwapToQwallet(const char* nodeIp, int nodePort,
                          const char* seed,
                          uint64_t liquidId,
                          const char* inputTokenInfo,
                          uint64_t inputAmount,
                          uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SwapToQwallet_input stqi;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_SWAP_TO_QWALLET;
    packet.transaction.inputSize = sizeof(SwapToQwallet_input);

    // Parse input token info and fill the input
    std::string str(inputTokenInfo);
    std::stringstream ss(str);
    std::string token;
    std::vector<std::string> fields;
    
    while (std::getline(ss, token, ',')) {
        fields.push_back(token);
    }
    
    if (fields.size() == 2) {
        memcpy(&packet.stqi.inputTokenInfo.assetName, fields[0].c_str(), 8);
        getPublicKeyFromIdentity(fields[1].c_str(), packet.stqi.inputTokenInfo.issuer);
    }
    
    packet.stqi.liquidId = liquidId;
    packet.stqi.inputAmount = inputAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapToQwallet_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(SwapToQwallet_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapToQwallet_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.stqi));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidSwapFromQwallet(const char* nodeIp, int nodePort,
                            const char* seed,
                            uint64_t liquidId,
                            const char* outputTokenInfo,
                            uint64_t qwalletAmount,
                            uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SwapFromQwallet_input sfqi;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_SWAP_FROM_QWALLET;
    packet.transaction.inputSize = sizeof(SwapFromQwallet_input);

    // Parse output token info and fill the input
    std::string str(outputTokenInfo);
    std::stringstream ss(str);
    std::string token;
    std::vector<std::string> fields;
    
    while (std::getline(ss, token, ',')) {
        fields.push_back(token);
    }
    
    if (fields.size() == 2) {
        memcpy(&packet.sfqi.outputTokenInfo.assetName, fields[0].c_str(), 8);
        getPublicKeyFromIdentity(fields[1].c_str(), packet.sfqi.outputTokenInfo.issuer);
    }
    
    packet.sfqi.liquidId = liquidId;
    packet.sfqi.qwalletAmount = qwalletAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapFromQwallet_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(SwapFromQwallet_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapFromQwallet_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.sfqi));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidSwapQUToQwallet(const char* nodeIp, int nodePort,
                            const char* seed,
                            uint64_t liquidId,
                            uint64_t quAmount,
                            uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SwapQUToQwallet_input sqtqi;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_SWAP_QU_TO_QWALLET;
    packet.transaction.inputSize = sizeof(SwapQUToQwallet_input);

    // Fill the input
    packet.sqtqi.liquidId = liquidId;
    packet.sqtqi.quAmount = quAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapQUToQwallet_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(SwapQUToQwallet_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapQUToQwallet_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.sqtqi));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidSwapQwalletToQU(const char* nodeIp, int nodePort,
                            const char* seed,
                            uint64_t liquidId,
                            uint64_t qwalletAmount,
                            uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SwapQwalletToQU_input sqtqi;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_SWAP_QWALLET_TO_QU;
    packet.transaction.inputSize = sizeof(SwapQwalletToQU_input);

    // Fill the input
    packet.sqtqi.liquidId = liquidId;
    packet.sqtqi.qwalletAmount = qwalletAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapQwalletToQU_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(SwapQwalletToQU_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SwapQwalletToQU_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.sqtqi));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidSingleSwap(const char* nodeIp, int nodePort,
                       const char* seed,
                       uint64_t liquidId,
                       const char* inputTokenInfo,
                       const char* outputTokenInfo,
                       uint64_t inputAmount,
                       uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        SingleSwap_input ssi;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_SINGLE_SWAP;
    packet.transaction.inputSize = sizeof(SingleSwap_input);

    // Parse input and output token info and fill the input
    std::string inputStr(inputTokenInfo);
    std::stringstream inputSS(inputStr);
    std::string inputToken;
    std::vector<std::string> inputFields;
    
    while (std::getline(inputSS, inputToken, ',')) {
        inputFields.push_back(inputToken);
    }
    
    if (inputFields.size() == 2) {
        memcpy(&packet.ssi.inputTokenInfo.assetName, inputFields[0].c_str(), 8);
        getPublicKeyFromIdentity(inputFields[1].c_str(), packet.ssi.inputTokenInfo.issuer);
    }

    std::string outputStr(outputTokenInfo);
    std::stringstream outputSS(outputStr);
    std::string outputToken;
    std::vector<std::string> outputFields;
    
    while (std::getline(outputSS, outputToken, ',')) {
        outputFields.push_back(outputToken);
    }
    
    if (outputFields.size() == 2) {
        memcpy(&packet.ssi.outputTokenInfo.assetName, outputFields[0].c_str(), 8);
        getPublicKeyFromIdentity(outputFields[1].c_str(), packet.ssi.outputTokenInfo.issuer);
    }
    
    packet.ssi.liquidId = liquidId;
    packet.ssi.inputAmount = inputAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SingleSwap_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(SingleSwap_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(SingleSwap_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.ssi));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidCrossSwap(const char* nodeIp, int nodePort,
                      const char* seed,
                      uint64_t liquidIdA,
                      const char* inputTokenInfoA,
                      uint64_t inputAmountA,
                      uint64_t liquidIdB,
                      const char* outputTokenInfoB,
                      uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        CrossSwap_input csi;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_CROSS_SWAP;
    packet.transaction.inputSize = sizeof(CrossSwap_input);

    // Parse input token info A and fill the input
    std::string inputStrA(inputTokenInfoA);
    std::stringstream inputSSA(inputStrA);
    std::string inputTokenA;
    std::vector<std::string> inputFieldsA;
    
    while (std::getline(inputSSA, inputTokenA, ',')) {
        inputFieldsA.push_back(inputTokenA);
    }
    
    if (inputFieldsA.size() == 2) {
        memcpy(&packet.csi.inputTokenInfoA.assetName, inputFieldsA[0].c_str(), 8);
        getPublicKeyFromIdentity(inputFieldsA[1].c_str(), packet.csi.inputTokenInfoA.issuer);
    }

    // Parse output token info B and fill the input
    std::string outputStrB(outputTokenInfoB);
    std::stringstream outputSSB(outputStrB);
    std::string outputTokenB;
    std::vector<std::string> outputFieldsB;
    
    while (std::getline(outputSSB, outputTokenB, ',')) {
        outputFieldsB.push_back(outputTokenB);
    }
    
    if (outputFieldsB.size() == 2) {
        memcpy(&packet.csi.outputTokenInfoB.assetName, outputFieldsB[0].c_str(), 8);
        getPublicKeyFromIdentity(outputFieldsB[1].c_str(), packet.csi.outputTokenInfoB.issuer);
    }
    
    packet.csi.liquidIdA = liquidIdA;
    packet.csi.inputAmountA = inputAmountA;
    packet.csi.liquidIdB = liquidIdB;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(CrossSwap_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(CrossSwap_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(CrossSwap_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.csi));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidInitializeStakingPool(const char* nodeIp, int nodePort,
                                  const char* seed,
                                  uint64_t liquidId,
                                  const char* bonusTokenInfo,
                                  uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        InitializeStakingPool_input ispi;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_INITIALIZE_STAKING_POOL;
    packet.transaction.inputSize = sizeof(InitializeStakingPool_input);

    // Parse bonus token info and fill the input
    std::string bonusStr(bonusTokenInfo);
    std::stringstream bonusSS(bonusStr);
    std::string bonusToken;
    std::vector<std::string> bonusFields;
    
    while (std::getline(bonusSS, bonusToken, ',')) {
        bonusFields.push_back(bonusToken);
    }
    
    if (bonusFields.size() == 2) {
        memcpy(&packet.ispi.bonusTokenInfo.assetName, bonusFields[0].c_str(), 8);
        getPublicKeyFromIdentity(bonusFields[1].c_str(), packet.ispi.bonusTokenInfo.issuer);
    }
    
    packet.ispi.liquidId = liquidId;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(InitializeStakingPool_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(InitializeStakingPool_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(InitializeStakingPool_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.ispi));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidDepositeBonusToken(const char* nodeIp, int nodePort,
                               const char* seed,
                               uint64_t liquidId,
                               uint64_t bonusTokenAmount,
                               uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        DepositeBonusToken_input dbti;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_DEPOSITE_BONUS_TOKEN;
    packet.transaction.inputSize = sizeof(DepositeBonusToken_input);

    // Fill the input
    packet.dbti.liquidId = liquidId;
    packet.dbti.bonusTokenAmount = bonusTokenAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(DepositeBonusToken_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(DepositeBonusToken_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(DepositeBonusToken_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.dbti));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidStake(const char* nodeIp, int nodePort,
                  const char* seed,
                  uint64_t liquidId,
                  uint64_t lpAmount,
                  uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        Stake_input si;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_STAKE;
    packet.transaction.inputSize = sizeof(Stake_input);

    // Fill the input
    packet.si.liquidId = liquidId;
    packet.si.lpAmount = lpAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(Stake_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(Stake_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(Stake_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.si));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}

void vliquidUnstake(const char* nodeIp, int nodePort,
                    const char* seed,
                    uint64_t liquidId,
                    uint64_t lpAmount,
                    uint32_t scheduledTickOffset) {
    auto qc = make_qc(nodeIp, nodePort);
    // Add signing-related variables
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};

    // Generate keys from seed
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);

    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        Unstake_input ui;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;

    // Set up transaction
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000; // Set appropriate fee
    uint32_t currentTick = getTickNumberFromNode(qc);
    uint32_t scheduledTick = currentTick + scheduledTickOffset;
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_UNSTAKE;
    packet.transaction.inputSize = sizeof(Unstake_input);

    // Fill the input
    packet.ui.liquidId = liquidId;
    packet.ui.lpAmount = lpAmount;

    // Sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(Unstake_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);

    // Set header
    packet.header.setSize(sizeof(packet.header) + sizeof(Transaction) + sizeof(Unstake_input) + SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    // Send transaction
    qc->sendData((uint8_t *) &packet, packet.header.size());
    
    // Generate transaction hash
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(Unstake_input) + SIGNATURE_SIZE,
                   digest,
                   32);
    getTxHashFromDigest(digest, txHash);

    // Log transaction details
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.ui));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");
}


void vliquidIssueAsset(const char* nodeIp, int nodePort,
                  const char* seed,
                  const char* assetName,
                  const char* unitOfMeasurement,
                  int64_t numberOfUnits,
                  char numberOfDecimalPlaces,
                  uint32_t scheduledTickOffset)
{
    auto qc = make_qc(nodeIp, nodePort);
    char assetNameS1[8] = {0};
    char UoMS1[8] = {0};
    memcpy(assetNameS1, assetName, strlen(assetName));
    for (int i = 0; i < 7; i++) UoMS1[i] = unitOfMeasurement[i] - 48;
    uint8_t privateKey[32] = {0};
    uint8_t sourcePublicKey[32] = {0};
    uint8_t destPublicKey[32] = {0};
    uint8_t subSeed[32] = {0};
    uint8_t digest[32] = {0};
    uint8_t signature[64] = {0};
    char txHash[128] = {0};
    getSubseedFromSeed((uint8_t*)seed, subSeed);
    getPrivateKeyFromSubSeed(subSeed, privateKey);
    getPublicKeyFromPrivateKey(privateKey, sourcePublicKey);
    
    ((uint64_t*)destPublicKey)[0] = VLIQUID_CONTRACT_INDEX;
    ((uint64_t*)destPublicKey)[1] = 0;
    ((uint64_t*)destPublicKey)[2] = 0;
    ((uint64_t*)destPublicKey)[3] = 0;

    struct {
        RequestResponseHeader header;
        Transaction transaction;
        IssueAsset_input ia;
        uint8_t sig[SIGNATURE_SIZE];
    } packet;
    memcpy(packet.transaction.sourcePublicKey, sourcePublicKey, 32);
    memcpy(packet.transaction.destinationPublicKey, destPublicKey, 32);
    packet.transaction.amount = 1000000000;
    uint32_t scheduledTick = 0;
    if (scheduledTickOffset < 50000){
        uint32_t currentTick = getTickNumberFromNode(qc);
        scheduledTick = currentTick + scheduledTickOffset;
    } else {
        scheduledTick = scheduledTickOffset;
    }
    packet.transaction.tick = scheduledTick;
    packet.transaction.inputType = VLIQUID_ISSUE_ASSET;
    packet.transaction.inputSize = sizeof(IssueAsset_input);

    // fill the input
    memcpy(&packet.ia.name, assetNameS1, 8);
    memcpy(&packet.ia.unitOfMeasurement, UoMS1, 8);
    packet.ia.numberOfUnits = numberOfUnits;
    packet.ia.numberOfDecimalPlaces = numberOfDecimalPlaces;
    // sign the packet
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction) + sizeof(IssueAsset_input),
                   digest,
                   32);
    sign(subSeed, sourcePublicKey, digest, signature);
    memcpy(packet.sig, signature, SIGNATURE_SIZE);
    // set header
    packet.header.setSize(sizeof(packet.header)+sizeof(Transaction)+sizeof(IssueAsset_input)+ SIGNATURE_SIZE);
    packet.header.zeroDejavu();
    packet.header.setType(BROADCAST_TRANSACTION);

    qc->sendData((uint8_t *) &packet, packet.header.size());
    KangarooTwelve((unsigned char*)&packet.transaction,
                   sizeof(Transaction)+sizeof(IssueAsset_input)+ SIGNATURE_SIZE,
                   digest,
                   32); // recompute digest for txhash
    getTxHashFromDigest(digest, txHash);
    LOG("Transaction has been sent!\n");
    printReceipt(packet.transaction, txHash, reinterpret_cast<const uint8_t *>(&packet.ia));
    LOG("run ./qubic-cli [...] -checktxontick %u %s\n", scheduledTick, txHash);
    LOG("to check your tx confirmation status\n");

}
