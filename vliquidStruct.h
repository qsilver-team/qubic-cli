#pragma once

#include <cstdint>
#include <cstring>

#define MAX_TOKENS 5

struct ConvertToMicroToken_input {
    uint8_t issuer[32];
    uint64_t assetName;
    int64_t expensiveTokenAmount;
};

struct ConvertToMicroToken_output {
    uint64_t microTokenAmount;
};

struct ConvertToExpensiveToken_input {
    uint8_t issuer[32];
    uint64_t assetName;
    uint64_t microTokenAmount;
};

struct ConvertToExpensiveToken_output {
    int64_t expensiveTokenAmount;
};

struct TransferMicroToken_input {
    uint8_t issuer[32];
    uint64_t assetName;
    uint8_t recipient[32];
    uint64_t microTokenAmount;
};

struct TransferMicroToken_output {
    uint64_t transferredMicroTokenAmount;
};

struct MicroTokenAllowance_input {
    uint8_t issuer[32];
    uint64_t assetName;
    uint8_t recipient[32];
    uint8_t spender[32];
};

struct MicroTokenAllowance_output {
    uint64_t balance;
};

struct BalanceOfMicroToken_input {
    uint8_t issuer[32];
    uint64_t assetName;
    uint8_t owner[32];
};

struct BalanceOfMicroToken_output {
    uint64_t balance;
};

struct ApproveMicroToken_input {
    uint8_t issuer[32];
    uint8_t recipient[32];
    uint64_t assetName;
    uint64_t microTokenAmount;
};

struct ApproveMicroToken_output {
    uint64_t approvedMicroTokenAmount;
};

struct TransferFromMicroToken_input {
    uint8_t issuer[32];
    uint64_t assetName;
    uint8_t spender[32];
    uint8_t recipient[32];
    uint64_t microTokenAmount;
};

struct TransferFromMicroToken_output {
    uint64_t transferredMicroTokenAmount;
};

struct TokenInfo {
    uint8_t issuer[32];
    uint64_t assetName;
    bool isMicroToken;
};

struct Token {
    TokenInfo tokenInfo;
    uint64_t balance;
    uint8_t weight;
};

struct CreateLiquid_input {
    Token tokens[MAX_TOKENS];
    uint8_t tokenLength;
    uint64_t quShares;
    uint8_t quWeight;
    uint16_t initialLiquid;
    uint8_t feeRate;
};

struct CreateLiquid_output {
    uint64_t liquidId;
};

struct AddLiquid_input {
    uint64_t tokenContribution;
    uint64_t liquidId;
};

struct AddLiquid_output {
    uint64_t addedContribution;
};

struct RemoveLiquid_input {
    uint64_t tokenContribution;
    uint64_t liquidId;
};

struct RemoveLiquid_output {
    uint64_t removedContribution;
};

struct SwapToQU_input {
    uint64_t liquidId;
    TokenInfo inputTokenInfo;
    uint64_t inputAmount;
};

struct SwapToQU_output {
    uint64_t quAmount;
};

struct SwapFromQU_input {
    uint64_t liquidId;
    TokenInfo outputTokenInfo;
    uint64_t quAmount;
};

struct SwapFromQU_output {
    uint64_t outputAmount;
};

struct SwapToQwallet_input {
    uint64_t liquidId;
    TokenInfo inputTokenInfo;
    uint64_t inputAmount;
};

struct SwapToQwallet_output {
    uint64_t qwalletAmount;
    uint64_t quAmount;
};

struct SwapFromQwallet_input {
    uint64_t liquidId;
    TokenInfo outputTokenInfo;
    uint64_t qwalletAmount;
};

struct SwapFromQwallet_output {
    uint64_t outputAmount;
    uint64_t quAmount;
};

struct SwapQUToQwallet_input {
    uint64_t liquidId;
    uint64_t quAmount;
};

struct SwapQUToQwallet_output {
    uint64_t qwalletAmount;
};

struct SwapQwalletToQU_input {
    uint64_t liquidId;
    uint64_t qwalletAmount;
};

struct SwapQwalletToQU_output {
    uint64_t quAmount;
};

struct SingleSwap_input {
    uint64_t liquidId;
    TokenInfo inputTokenInfo;
    TokenInfo outputTokenInfo;
    uint64_t inputAmount;
};

struct SingleSwap_output {
    uint64_t outputAmount;
    uint64_t outputQwalletAmount;
    uint64_t outputQuAmount;
};

struct CrossSwap_input {
    uint64_t liquidIdA;
    TokenInfo inputTokenInfoA;
    uint64_t inputAmountA;
    uint64_t liquidIdB;
    TokenInfo outputTokenInfoB;
};

struct CrossSwap_output {
    uint64_t outputAmountB;
    uint64_t outputQwalletAmount;
    uint64_t outputQuAmount;
};

struct InitializeStakingPool_input {
    uint64_t liquidId;
    TokenInfo bonusTokenInfo;
};

struct InitializeStakingPool_output {
    bool success;
};

struct DepositeBonusToken_input {
    uint64_t liquidId;
    uint64_t bonusTokenAmount;
};

struct DepositeBonusToken_output {
    uint64_t depositedBonusTokenAmount;
};

struct Stake_input {
    uint64_t liquidId;
    uint64_t lpAmount;
};

struct Stake_output {
    uint64_t stakedLpAmount;
};

struct Unstake_input {
    uint64_t liquidId;
    uint64_t lpAmount;
};

struct Unstake_output {
    uint64_t unstakedLpAmount;
};

struct IssueAsset_input
{
    uint64_t name;
    int64_t numberOfUnits;
    uint64_t unitOfMeasurement;
    char numberOfDecimalPlaces;
};
