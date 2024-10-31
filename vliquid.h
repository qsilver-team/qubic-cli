#pragma once


struct ExamPublic_input
{
    uint64_t inputValue;
};
struct ExamPublic_output
{
    uint64_t outputValue;
};

void qxExamPublic(const char* nodeIp, int nodePort,
                         const char* offset);