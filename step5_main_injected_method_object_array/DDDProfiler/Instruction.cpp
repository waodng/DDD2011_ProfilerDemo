#include "StdAfx.h"
#include "Instruction.h"

Instruction::Instruction(void)
{
    m_operation = CEE_NOP;
    m_operand = 0;
    m_offset = -1;
    m_isBranch = false;
    m_origOffset = -1;
}

Instruction::Instruction(CanonicalName operation, ULONGLONG operand)
{
    m_operation = operation;
    m_operand = operand;
    m_offset = -1;
    m_isBranch = false;
    m_origOffset = -1;
}

Instruction::Instruction(CanonicalName operation)
{
    m_operation = operation;
    m_operand = 0;
    m_offset = -1;
    m_isBranch = false;
    m_origOffset = -1;
}

Instruction::Instruction(CanonicalName operation, Instruction *branchLocation)
{
    m_operation = operation;
    m_operand = 0;
    m_offset = -1;
    m_isBranch = true;
    m_origOffset = -1;
    // Seems like we can just push 0 here, it's the m_branches that gets used?!?! 
    // But if we don't add it something to m_branchOffsets, it'll throw an exception!!!
    m_branchOffsets.push_back(0);
    m_branches.push_back(branchLocation);
}

Instruction::~Instruction(void)
{
}

Instruction& Instruction::operator = (const Instruction& b)
{
    m_offset = b.m_offset;
    m_operation = b.m_operation;
    m_operand = b.m_operand;
    m_isBranch = b.m_isBranch;
    m_branchOffsets = b.m_branchOffsets;
    m_branches = b.m_branches;
    m_origOffset = b.m_origOffset;
    return *this;
}


