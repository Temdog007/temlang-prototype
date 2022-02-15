#pragma once

#include "Expression.h"
#include "InstructionList.h"
#include "List.h"
#include "MatchBranchType.h"

typedef struct Branch
{
    MatchBranchType type;
    union
    {
        Expression expression;
        InstructionList instructions;
    };
} Branch, *pBranch;

static inline void
BranchFree(Branch*);

static inline bool
BranchCopy(Branch*, const Branch*, const Allocator*);

static inline TemLangString
BranchToString(const Branch*, const Allocator*);

typedef struct MatchBranch
{
    Expression matcher;
    Branch branch;
} MatchBranch, *pMatchBranch;

static inline void
MatchBranchFree(MatchBranch*);

static inline bool
MatchBranchCopy(MatchBranch*, const MatchBranch*, const Allocator*);

static inline TemLangString
MatchBranchToString(const MatchBranch*, const Allocator*);

MAKE_LIST(MatchBranch);

typedef struct MatchExpression
{
    Expression matcher;
    TemLangStringList captures;
    MatchBranchList branches;
    Branch defaultBranch;
} MatchExpression, *pMatchExpression;

static inline void
MatchExpressionFree(MatchExpression*);

static inline bool
MatchExpressionCopy(MatchExpression*, const MatchExpression*, const Allocator*);

static inline TemLangString
MatchExpressionToString(const MatchExpression*, const Allocator*);

static inline bool
EvaluateMatchExpression(const MatchExpression*,
                        State*,
                        pValue,
                        const Allocator*);

DEFAULT_MAKE_LIST_FUNCTIONS(MatchBranch);