
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum InstructionType
{
    InstructionType_Invalid = -1,
    InstructionType_Return,
    InstructionType_IfReturn,
    InstructionType_While,
    InstructionType_Until,
    InstructionType_Iterate,
    InstructionType_Run,
    InstructionType_Print,
    InstructionType_Error,
    InstructionType_Match,
    InstructionType_Format,
    InstructionType_Verify,
    InstructionType_CreateVariable,
    InstructionType_UpdateVariable,
    InstructionType_ConvertContainer,
    InstructionType_Inline,
    InstructionType_InlineC,
    InstructionType_InlineCFunction,
    InstructionType_InlineCFunctionReturnStruct,
    InstructionType_InlineCHeaders,
    InstructionType_InlineCFile,
    InstructionType_InlineFile,
    InstructionType_InlineVariable,
    InstructionType_DefineRange,
    InstructionType_DefineStruct,
    InstructionType_DefineEnum,
    InstructionType_DefineFunction,
    InstructionType_InlineData,
    InstructionType_InlineText,
    InstructionType_NoCompile,
    InstructionType_NoCleanup,
    InstructionType_ChangeFlag,
    InstructionType_SetAllFlag,
    InstructionType_ListModify,
    InstructionType_NumberRound
} InstructionType,
  *pInstructionType;

#define InstructionTypeCount 34
#define InstructionTypeLongestString 27

static const InstructionType InstructionTypeMembers[] = {
    InstructionType_Return,
    InstructionType_IfReturn,
    InstructionType_While,
    InstructionType_Until,
    InstructionType_Iterate,
    InstructionType_Run,
    InstructionType_Print,
    InstructionType_Error,
    InstructionType_Match,
    InstructionType_Format,
    InstructionType_Verify,
    InstructionType_CreateVariable,
    InstructionType_UpdateVariable,
    InstructionType_ConvertContainer,
    InstructionType_Inline,
    InstructionType_InlineC,
    InstructionType_InlineCFunction,
    InstructionType_InlineCFunctionReturnStruct,
    InstructionType_InlineCHeaders,
    InstructionType_InlineCFile,
    InstructionType_InlineFile,
    InstructionType_InlineVariable,
    InstructionType_DefineRange,
    InstructionType_DefineStruct,
    InstructionType_DefineEnum,
    InstructionType_DefineFunction,
    InstructionType_InlineData,
    InstructionType_InlineText,
    InstructionType_NoCompile,
    InstructionType_NoCleanup,
    InstructionType_ChangeFlag,
    InstructionType_SetAllFlag,
    InstructionType_ListModify,
    InstructionType_NumberRound
};

static inline InstructionType
InstructionTypeFromIndex(size_t index)
{
    if (index >= InstructionTypeCount) {
        return InstructionType_Invalid;
    }
    return InstructionTypeMembers[index];
}
static inline InstructionType
InstructionTypeFromString(const void* c, const size_t size)
{
    if (size > InstructionTypeLongestString) {
        return InstructionType_Invalid;
    }
    if (size == 6 && memcmp("Return", c, 6) == 0) {
        return InstructionType_Return;
    }
    if (size == 8 && memcmp("IfReturn", c, 8) == 0) {
        return InstructionType_IfReturn;
    }
    if (size == 5 && memcmp("While", c, 5) == 0) {
        return InstructionType_While;
    }
    if (size == 5 && memcmp("Until", c, 5) == 0) {
        return InstructionType_Until;
    }
    if (size == 7 && memcmp("Iterate", c, 7) == 0) {
        return InstructionType_Iterate;
    }
    if (size == 3 && memcmp("Run", c, 3) == 0) {
        return InstructionType_Run;
    }
    if (size == 5 && memcmp("Print", c, 5) == 0) {
        return InstructionType_Print;
    }
    if (size == 5 && memcmp("Error", c, 5) == 0) {
        return InstructionType_Error;
    }
    if (size == 5 && memcmp("Match", c, 5) == 0) {
        return InstructionType_Match;
    }
    if (size == 6 && memcmp("Format", c, 6) == 0) {
        return InstructionType_Format;
    }
    if (size == 6 && memcmp("Verify", c, 6) == 0) {
        return InstructionType_Verify;
    }
    if (size == 14 && memcmp("CreateVariable", c, 14) == 0) {
        return InstructionType_CreateVariable;
    }
    if (size == 14 && memcmp("UpdateVariable", c, 14) == 0) {
        return InstructionType_UpdateVariable;
    }
    if (size == 16 && memcmp("ConvertContainer", c, 16) == 0) {
        return InstructionType_ConvertContainer;
    }
    if (size == 6 && memcmp("Inline", c, 6) == 0) {
        return InstructionType_Inline;
    }
    if (size == 7 && memcmp("InlineC", c, 7) == 0) {
        return InstructionType_InlineC;
    }
    if (size == 15 && memcmp("InlineCFunction", c, 15) == 0) {
        return InstructionType_InlineCFunction;
    }
    if (size == 27 && memcmp("InlineCFunctionReturnStruct", c, 27) == 0) {
        return InstructionType_InlineCFunctionReturnStruct;
    }
    if (size == 14 && memcmp("InlineCHeaders", c, 14) == 0) {
        return InstructionType_InlineCHeaders;
    }
    if (size == 11 && memcmp("InlineCFile", c, 11) == 0) {
        return InstructionType_InlineCFile;
    }
    if (size == 10 && memcmp("InlineFile", c, 10) == 0) {
        return InstructionType_InlineFile;
    }
    if (size == 14 && memcmp("InlineVariable", c, 14) == 0) {
        return InstructionType_InlineVariable;
    }
    if (size == 11 && memcmp("DefineRange", c, 11) == 0) {
        return InstructionType_DefineRange;
    }
    if (size == 12 && memcmp("DefineStruct", c, 12) == 0) {
        return InstructionType_DefineStruct;
    }
    if (size == 10 && memcmp("DefineEnum", c, 10) == 0) {
        return InstructionType_DefineEnum;
    }
    if (size == 14 && memcmp("DefineFunction", c, 14) == 0) {
        return InstructionType_DefineFunction;
    }
    if (size == 10 && memcmp("InlineData", c, 10) == 0) {
        return InstructionType_InlineData;
    }
    if (size == 10 && memcmp("InlineText", c, 10) == 0) {
        return InstructionType_InlineText;
    }
    if (size == 9 && memcmp("NoCompile", c, 9) == 0) {
        return InstructionType_NoCompile;
    }
    if (size == 9 && memcmp("NoCleanup", c, 9) == 0) {
        return InstructionType_NoCleanup;
    }
    if (size == 10 && memcmp("ChangeFlag", c, 10) == 0) {
        return InstructionType_ChangeFlag;
    }
    if (size == 10 && memcmp("SetAllFlag", c, 10) == 0) {
        return InstructionType_SetAllFlag;
    }
    if (size == 10 && memcmp("ListModify", c, 10) == 0) {
        return InstructionType_ListModify;
    }
    if (size == 11 && memcmp("NumberRound", c, 11) == 0) {
        return InstructionType_NumberRound;
    }
    return InstructionType_Invalid;
}
static inline InstructionType
InstructionTypeFromCaseInsensitiveString(const char* original,
                                         const size_t size)
{
    if (size > InstructionTypeLongestString) {
        return InstructionType_Invalid;
    }
    char c[InstructionTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 6 && memcmp("return", c, 6) == 0) {
        return InstructionType_Return;
    }
    if (size == 8 && memcmp("ifreturn", c, 8) == 0) {
        return InstructionType_IfReturn;
    }
    if (size == 5 && memcmp("while", c, 5) == 0) {
        return InstructionType_While;
    }
    if (size == 5 && memcmp("until", c, 5) == 0) {
        return InstructionType_Until;
    }
    if (size == 7 && memcmp("iterate", c, 7) == 0) {
        return InstructionType_Iterate;
    }
    if (size == 3 && memcmp("run", c, 3) == 0) {
        return InstructionType_Run;
    }
    if (size == 5 && memcmp("print", c, 5) == 0) {
        return InstructionType_Print;
    }
    if (size == 5 && memcmp("error", c, 5) == 0) {
        return InstructionType_Error;
    }
    if (size == 5 && memcmp("match", c, 5) == 0) {
        return InstructionType_Match;
    }
    if (size == 6 && memcmp("format", c, 6) == 0) {
        return InstructionType_Format;
    }
    if (size == 6 && memcmp("verify", c, 6) == 0) {
        return InstructionType_Verify;
    }
    if (size == 14 && memcmp("createvariable", c, 14) == 0) {
        return InstructionType_CreateVariable;
    }
    if (size == 14 && memcmp("updatevariable", c, 14) == 0) {
        return InstructionType_UpdateVariable;
    }
    if (size == 16 && memcmp("convertcontainer", c, 16) == 0) {
        return InstructionType_ConvertContainer;
    }
    if (size == 6 && memcmp("inline", c, 6) == 0) {
        return InstructionType_Inline;
    }
    if (size == 7 && memcmp("inlinec", c, 7) == 0) {
        return InstructionType_InlineC;
    }
    if (size == 15 && memcmp("inlinecfunction", c, 15) == 0) {
        return InstructionType_InlineCFunction;
    }
    if (size == 27 && memcmp("inlinecfunctionreturnstruct", c, 27) == 0) {
        return InstructionType_InlineCFunctionReturnStruct;
    }
    if (size == 14 && memcmp("inlinecheaders", c, 14) == 0) {
        return InstructionType_InlineCHeaders;
    }
    if (size == 11 && memcmp("inlinecfile", c, 11) == 0) {
        return InstructionType_InlineCFile;
    }
    if (size == 10 && memcmp("inlinefile", c, 10) == 0) {
        return InstructionType_InlineFile;
    }
    if (size == 14 && memcmp("inlinevariable", c, 14) == 0) {
        return InstructionType_InlineVariable;
    }
    if (size == 11 && memcmp("definerange", c, 11) == 0) {
        return InstructionType_DefineRange;
    }
    if (size == 12 && memcmp("definestruct", c, 12) == 0) {
        return InstructionType_DefineStruct;
    }
    if (size == 10 && memcmp("defineenum", c, 10) == 0) {
        return InstructionType_DefineEnum;
    }
    if (size == 14 && memcmp("definefunction", c, 14) == 0) {
        return InstructionType_DefineFunction;
    }
    if (size == 10 && memcmp("inlinedata", c, 10) == 0) {
        return InstructionType_InlineData;
    }
    if (size == 10 && memcmp("inlinetext", c, 10) == 0) {
        return InstructionType_InlineText;
    }
    if (size == 9 && memcmp("nocompile", c, 9) == 0) {
        return InstructionType_NoCompile;
    }
    if (size == 9 && memcmp("nocleanup", c, 9) == 0) {
        return InstructionType_NoCleanup;
    }
    if (size == 10 && memcmp("changeflag", c, 10) == 0) {
        return InstructionType_ChangeFlag;
    }
    if (size == 10 && memcmp("setallflag", c, 10) == 0) {
        return InstructionType_SetAllFlag;
    }
    if (size == 10 && memcmp("listmodify", c, 10) == 0) {
        return InstructionType_ListModify;
    }
    if (size == 11 && memcmp("numberround", c, 11) == 0) {
        return InstructionType_NumberRound;
    }
    return InstructionType_Invalid;
}
static inline const char*
InstructionTypeToString(const InstructionType e)
{
    if (e == InstructionType_Return) {
        return "Return";
    }
    if (e == InstructionType_IfReturn) {
        return "IfReturn";
    }
    if (e == InstructionType_While) {
        return "While";
    }
    if (e == InstructionType_Until) {
        return "Until";
    }
    if (e == InstructionType_Iterate) {
        return "Iterate";
    }
    if (e == InstructionType_Run) {
        return "Run";
    }
    if (e == InstructionType_Print) {
        return "Print";
    }
    if (e == InstructionType_Error) {
        return "Error";
    }
    if (e == InstructionType_Match) {
        return "Match";
    }
    if (e == InstructionType_Format) {
        return "Format";
    }
    if (e == InstructionType_Verify) {
        return "Verify";
    }
    if (e == InstructionType_CreateVariable) {
        return "CreateVariable";
    }
    if (e == InstructionType_UpdateVariable) {
        return "UpdateVariable";
    }
    if (e == InstructionType_ConvertContainer) {
        return "ConvertContainer";
    }
    if (e == InstructionType_Inline) {
        return "Inline";
    }
    if (e == InstructionType_InlineC) {
        return "InlineC";
    }
    if (e == InstructionType_InlineCFunction) {
        return "InlineCFunction";
    }
    if (e == InstructionType_InlineCFunctionReturnStruct) {
        return "InlineCFunctionReturnStruct";
    }
    if (e == InstructionType_InlineCHeaders) {
        return "InlineCHeaders";
    }
    if (e == InstructionType_InlineCFile) {
        return "InlineCFile";
    }
    if (e == InstructionType_InlineFile) {
        return "InlineFile";
    }
    if (e == InstructionType_InlineVariable) {
        return "InlineVariable";
    }
    if (e == InstructionType_DefineRange) {
        return "DefineRange";
    }
    if (e == InstructionType_DefineStruct) {
        return "DefineStruct";
    }
    if (e == InstructionType_DefineEnum) {
        return "DefineEnum";
    }
    if (e == InstructionType_DefineFunction) {
        return "DefineFunction";
    }
    if (e == InstructionType_InlineData) {
        return "InlineData";
    }
    if (e == InstructionType_InlineText) {
        return "InlineText";
    }
    if (e == InstructionType_NoCompile) {
        return "NoCompile";
    }
    if (e == InstructionType_NoCleanup) {
        return "NoCleanup";
    }
    if (e == InstructionType_ChangeFlag) {
        return "ChangeFlag";
    }
    if (e == InstructionType_SetAllFlag) {
        return "SetAllFlag";
    }
    if (e == InstructionType_ListModify) {
        return "ListModify";
    }
    if (e == InstructionType_NumberRound) {
        return "NumberRound";
    }
    return "Invalid";
}