#pragma once

#include "vliquidStruct.h"

void vliquidBalanceOfMicroToken(const char* nodeIp, int nodePort,
                                const char* assetName,
                                const char* issuer,
                                const char* owner);

void vliquidMicroTokenAllowance(const char* nodeIp, int nodePort,
                                const char* assetName,
                                const char* issuer,
                                const char* recipient,
                                const char* spender);

void vliquidApproveMicroToken(const char* nodeIp, int nodePort,
                              const char* seed,
                              const char* assetName,
                              const char* issuer,
                              const char* recipient,
                              uint64_t microTokenAmount,
                              uint32_t scheduledTickOffset);

void vliquidTransferMicroToken(const char* nodeIp, int nodePort,
                               const char* seed,
                               const char* assetName,
                               const char* issuer,
                               const char* recipient,
                               uint64_t microTokenAmount,
                               uint32_t scheduledTickOffset);

void vliquidTransferFromMicroToken(const char* nodeIp, int nodePort,
                                   const char* seed,
                                   const char* assetName,
                                   const char* issuer,
                                   const char* spender,
                                   const char* recipient,
                                   uint64_t microTokenAmount,
                                   uint32_t scheduledTickOffset);

void vliquidConvertToMicroToken(const char* nodeIp, int nodePort,
                                const char* seed,
                                const char* assetName,
                                const char* issuer,
                                int64_t expensiveTokenAmount,
                                uint32_t scheduledTickOffset);

void vliquidConvertToExpensiveToken(const char* nodeIp, int nodePort,
                                    const char* seed,
                                    const char* assetName,
                                    const char* issuer,
                                    uint64_t microTokenAmount,
                                    uint32_t scheduledTickOffset);

void vliquidCreateLiquid(const char* nodeIp, int nodePort,
                         const char* seed,
                         const char* tokens,
                         uint8_t tokenLength,
                         uint64_t quShares,
                         uint8_t quWeight,
                         uint16_t initialLiquid,
                         uint8_t feeRate,
                         uint32_t scheduledTickOffset);

void vliquidAddLiquid(const char* nodeIp, int nodePort,
                      const char* seed,
                      uint64_t tokenContribution,
                      uint64_t liquidId,
                      uint32_t scheduledTickOffset);

void vliquidRemoveLiquid(const char* nodeIp, int nodePort,
                         const char* seed,
                         uint64_t tokenContribution,
                         uint64_t liquidId,
                         uint32_t scheduledTickOffset);

void vliquidSwapToQU(const char* nodeIp, int nodePort,
                     const char* seed,
                     uint64_t liquidId,
                     const char* inputTokenInfo,
                     uint64_t inputAmount,
                     uint32_t scheduledTickOffset);

void vliquidSwapFromQU(const char* nodeIp, int nodePort,
                       const char* seed,
                       uint64_t liquidId,
                       const char* outputTokenInfo,
                       uint64_t quAmount,
                       uint32_t scheduledTickOffset);

void vliquidSwapToQwallet(const char* nodeIp, int nodePort,
                          const char* seed,
                          uint64_t liquidId,
                          const char* inputTokenInfo,
                          uint64_t inputAmount,
                          uint32_t scheduledTickOffset);

void vliquidSwapFromQwallet(const char* nodeIp, int nodePort,
                            const char* seed,
                            uint64_t liquidId,
                            const char* outputTokenInfo,
                            uint64_t qwalletAmount,
                            uint32_t scheduledTickOffset);

void vliquidSwapQUToQwallet(const char* nodeIp, int nodePort,
                            const char* seed,
                            uint64_t liquidId,
                            uint64_t quAmount,
                            uint32_t scheduledTickOffset);

void vliquidSwapQwalletToQU(const char* nodeIp, int nodePort,
                            const char* seed,
                            uint64_t liquidId,
                            uint64_t qwalletAmount,
                            uint32_t scheduledTickOffset);

void vliquidSingleSwap(const char* nodeIp, int nodePort,
                       const char* seed,
                       uint64_t liquidId,
                       const char* inputTokenInfo,
                       const char* outputTokenInfo,
                       uint64_t inputAmount,
                       uint32_t scheduledTickOffset);

void vliquidCrossSwap(const char* nodeIp, int nodePort,
                      const char* seed,
                      uint64_t liquidIdA,
                      const char* inputTokenInfoA,
                      uint64_t inputAmountA,
                      uint64_t liquidIdB,
                      const char* outputTokenInfoB,
                      uint32_t scheduledTickOffset);

void vliquidInitializeStakingPool(const char* nodeIp, int nodePort,
                                  const char* seed,
                                  uint64_t liquidId,
                                  const char* bonusTokenInfo,
                                  uint32_t scheduledTickOffset);

void vliquidDepositeBonusToken(const char* nodeIp, int nodePort,
                               const char* seed,
                               uint64_t liquidId,
                               uint64_t bonusTokenAmount,
                               uint32_t scheduledTickOffset);

void vliquidStake(const char* nodeIp, int nodePort,
                  const char* seed,
                  uint64_t liquidId,
                  uint64_t lpAmount,
                  uint32_t scheduledTickOffset);

void vliquidUnstake(const char* nodeIp, int nodePort,
                    const char* seed,
                    uint64_t liquidId,
                    uint64_t lpAmount,
                    uint32_t scheduledTickOffset);

void vliquidExamPublic(const char* nodeIp, int nodePort,
                       uint64_t inputValue);

void parseTokensString(const char* tokensStr, uint8_t tokenLength, Token* tokens);
