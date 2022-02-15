#pragma once

#include "TemLangString.h"

typedef struct Instruction Instruction, *pInstruction;

static inline void
InstructionFree(Instruction*);

static inline bool
InstructionCopy(Instruction*, const Instruction*, const Allocator*);

static inline TemLangString
InstructionToString(const Instruction*, const Allocator*);

MAKE_LIST(Instruction);

static inline TemLangString
InstructionListToString(const InstructionList*, const Allocator*);
