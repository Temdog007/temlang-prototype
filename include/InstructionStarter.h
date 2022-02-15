
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum InstructionStarter
{
    InstructionStarter_Invalid = -1,
    InstructionStarter_Let,
    InstructionStarter_MLet,
    InstructionStarter_MutableLet,
    InstructionStarter_Constant,
    InstructionStarter_Const,
    InstructionStarter_Set,
    InstructionStarter_IfReturn,
    InstructionStarter_ReturnIf,
    InstructionStarter_Return,
    InstructionStarter_Run,
    InstructionStarter_Print,
    InstructionStarter_Error,
    InstructionStarter_Iterate,
    InstructionStarter_Format,
    InstructionStarter_Nullary,
    InstructionStarter_Unary,
    InstructionStarter_Binary,
    InstructionStarter_Procedure,
    InstructionStarter_While,
    InstructionStarter_Until,
    InstructionStarter_Match,
    InstructionStarter_NoCompile,
    InstructionStarter_NoCleanup,
    InstructionStarter_On,
    InstructionStarter_Off,
    InstructionStarter_Toggle,
    InstructionStarter_Clear,
    InstructionStarter_All,
    InstructionStarter_ToArray,
    InstructionStarter_ToList,
    InstructionStarter_Verify,
    InstructionStarter_Floor,
    InstructionStarter_Round,
    InstructionStarter_Ceil,
    InstructionStarter_Append,
    InstructionStarter_Insert,
    InstructionStarter_Remove,
    InstructionStarter_SwapRemove,
    InstructionStarter_Pop,
    InstructionStarter_Empty,
    InstructionStarter_Inline,
    InstructionStarter_InlineC,
    InstructionStarter_InlineCFunction,
    InstructionStarter_InlineCFunctionReturnStruct,
    InstructionStarter_InlineCHeaders,
    InstructionStarter_InlineCFile,
    InstructionStarter_InlineFile,
    InstructionStarter_InlineVariable,
    InstructionStarter_InlineData,
    InstructionStarter_InlineText,
    InstructionStarter_Range,
    InstructionStarter_Struct,
    InstructionStarter_Resource,
    InstructionStarter_Variant,
    InstructionStarter_Enum,
    InstructionStarter_Flag
} InstructionStarter,
  *pInstructionStarter;

#define InstructionStarterCount 56
#define InstructionStarterLongestString 27

static const InstructionStarter InstructionStarterMembers[] = {
    InstructionStarter_Let,
    InstructionStarter_MLet,
    InstructionStarter_MutableLet,
    InstructionStarter_Constant,
    InstructionStarter_Const,
    InstructionStarter_Set,
    InstructionStarter_IfReturn,
    InstructionStarter_ReturnIf,
    InstructionStarter_Return,
    InstructionStarter_Run,
    InstructionStarter_Print,
    InstructionStarter_Error,
    InstructionStarter_Iterate,
    InstructionStarter_Format,
    InstructionStarter_Nullary,
    InstructionStarter_Unary,
    InstructionStarter_Binary,
    InstructionStarter_Procedure,
    InstructionStarter_While,
    InstructionStarter_Until,
    InstructionStarter_Match,
    InstructionStarter_NoCompile,
    InstructionStarter_NoCleanup,
    InstructionStarter_On,
    InstructionStarter_Off,
    InstructionStarter_Toggle,
    InstructionStarter_Clear,
    InstructionStarter_All,
    InstructionStarter_ToArray,
    InstructionStarter_ToList,
    InstructionStarter_Verify,
    InstructionStarter_Floor,
    InstructionStarter_Round,
    InstructionStarter_Ceil,
    InstructionStarter_Append,
    InstructionStarter_Insert,
    InstructionStarter_Remove,
    InstructionStarter_SwapRemove,
    InstructionStarter_Pop,
    InstructionStarter_Empty,
    InstructionStarter_Inline,
    InstructionStarter_InlineC,
    InstructionStarter_InlineCFunction,
    InstructionStarter_InlineCFunctionReturnStruct,
    InstructionStarter_InlineCHeaders,
    InstructionStarter_InlineCFile,
    InstructionStarter_InlineFile,
    InstructionStarter_InlineVariable,
    InstructionStarter_InlineData,
    InstructionStarter_InlineText,
    InstructionStarter_Range,
    InstructionStarter_Struct,
    InstructionStarter_Resource,
    InstructionStarter_Variant,
    InstructionStarter_Enum,
    InstructionStarter_Flag
};

static inline InstructionStarter
InstructionStarterFromIndex(size_t index)
{
    if (index >= InstructionStarterCount) {
        return InstructionStarter_Invalid;
    }
    return InstructionStarterMembers[index];
}
static inline InstructionStarter
InstructionStarterFromString(const void* c, const size_t size)
{
    if (size > InstructionStarterLongestString) {
        return InstructionStarter_Invalid;
    }
    if (size == 3 && memcmp("Let", c, 3) == 0) {
        return InstructionStarter_Let;
    }
    if (size == 4 && memcmp("MLet", c, 4) == 0) {
        return InstructionStarter_MLet;
    }
    if (size == 10 && memcmp("MutableLet", c, 10) == 0) {
        return InstructionStarter_MutableLet;
    }
    if (size == 8 && memcmp("Constant", c, 8) == 0) {
        return InstructionStarter_Constant;
    }
    if (size == 5 && memcmp("Const", c, 5) == 0) {
        return InstructionStarter_Const;
    }
    if (size == 3 && memcmp("Set", c, 3) == 0) {
        return InstructionStarter_Set;
    }
    if (size == 8 && memcmp("IfReturn", c, 8) == 0) {
        return InstructionStarter_IfReturn;
    }
    if (size == 8 && memcmp("ReturnIf", c, 8) == 0) {
        return InstructionStarter_ReturnIf;
    }
    if (size == 6 && memcmp("Return", c, 6) == 0) {
        return InstructionStarter_Return;
    }
    if (size == 3 && memcmp("Run", c, 3) == 0) {
        return InstructionStarter_Run;
    }
    if (size == 5 && memcmp("Print", c, 5) == 0) {
        return InstructionStarter_Print;
    }
    if (size == 5 && memcmp("Error", c, 5) == 0) {
        return InstructionStarter_Error;
    }
    if (size == 7 && memcmp("Iterate", c, 7) == 0) {
        return InstructionStarter_Iterate;
    }
    if (size == 6 && memcmp("Format", c, 6) == 0) {
        return InstructionStarter_Format;
    }
    if (size == 7 && memcmp("Nullary", c, 7) == 0) {
        return InstructionStarter_Nullary;
    }
    if (size == 5 && memcmp("Unary", c, 5) == 0) {
        return InstructionStarter_Unary;
    }
    if (size == 6 && memcmp("Binary", c, 6) == 0) {
        return InstructionStarter_Binary;
    }
    if (size == 9 && memcmp("Procedure", c, 9) == 0) {
        return InstructionStarter_Procedure;
    }
    if (size == 5 && memcmp("While", c, 5) == 0) {
        return InstructionStarter_While;
    }
    if (size == 5 && memcmp("Until", c, 5) == 0) {
        return InstructionStarter_Until;
    }
    if (size == 5 && memcmp("Match", c, 5) == 0) {
        return InstructionStarter_Match;
    }
    if (size == 9 && memcmp("NoCompile", c, 9) == 0) {
        return InstructionStarter_NoCompile;
    }
    if (size == 9 && memcmp("NoCleanup", c, 9) == 0) {
        return InstructionStarter_NoCleanup;
    }
    if (size == 2 && memcmp("On", c, 2) == 0) {
        return InstructionStarter_On;
    }
    if (size == 3 && memcmp("Off", c, 3) == 0) {
        return InstructionStarter_Off;
    }
    if (size == 6 && memcmp("Toggle", c, 6) == 0) {
        return InstructionStarter_Toggle;
    }
    if (size == 5 && memcmp("Clear", c, 5) == 0) {
        return InstructionStarter_Clear;
    }
    if (size == 3 && memcmp("All", c, 3) == 0) {
        return InstructionStarter_All;
    }
    if (size == 7 && memcmp("ToArray", c, 7) == 0) {
        return InstructionStarter_ToArray;
    }
    if (size == 6 && memcmp("ToList", c, 6) == 0) {
        return InstructionStarter_ToList;
    }
    if (size == 6 && memcmp("Verify", c, 6) == 0) {
        return InstructionStarter_Verify;
    }
    if (size == 5 && memcmp("Floor", c, 5) == 0) {
        return InstructionStarter_Floor;
    }
    if (size == 5 && memcmp("Round", c, 5) == 0) {
        return InstructionStarter_Round;
    }
    if (size == 4 && memcmp("Ceil", c, 4) == 0) {
        return InstructionStarter_Ceil;
    }
    if (size == 6 && memcmp("Append", c, 6) == 0) {
        return InstructionStarter_Append;
    }
    if (size == 6 && memcmp("Insert", c, 6) == 0) {
        return InstructionStarter_Insert;
    }
    if (size == 6 && memcmp("Remove", c, 6) == 0) {
        return InstructionStarter_Remove;
    }
    if (size == 10 && memcmp("SwapRemove", c, 10) == 0) {
        return InstructionStarter_SwapRemove;
    }
    if (size == 3 && memcmp("Pop", c, 3) == 0) {
        return InstructionStarter_Pop;
    }
    if (size == 5 && memcmp("Empty", c, 5) == 0) {
        return InstructionStarter_Empty;
    }
    if (size == 6 && memcmp("Inline", c, 6) == 0) {
        return InstructionStarter_Inline;
    }
    if (size == 7 && memcmp("InlineC", c, 7) == 0) {
        return InstructionStarter_InlineC;
    }
    if (size == 15 && memcmp("InlineCFunction", c, 15) == 0) {
        return InstructionStarter_InlineCFunction;
    }
    if (size == 27 && memcmp("InlineCFunctionReturnStruct", c, 27) == 0) {
        return InstructionStarter_InlineCFunctionReturnStruct;
    }
    if (size == 14 && memcmp("InlineCHeaders", c, 14) == 0) {
        return InstructionStarter_InlineCHeaders;
    }
    if (size == 11 && memcmp("InlineCFile", c, 11) == 0) {
        return InstructionStarter_InlineCFile;
    }
    if (size == 10 && memcmp("InlineFile", c, 10) == 0) {
        return InstructionStarter_InlineFile;
    }
    if (size == 14 && memcmp("InlineVariable", c, 14) == 0) {
        return InstructionStarter_InlineVariable;
    }
    if (size == 10 && memcmp("InlineData", c, 10) == 0) {
        return InstructionStarter_InlineData;
    }
    if (size == 10 && memcmp("InlineText", c, 10) == 0) {
        return InstructionStarter_InlineText;
    }
    if (size == 5 && memcmp("Range", c, 5) == 0) {
        return InstructionStarter_Range;
    }
    if (size == 6 && memcmp("Struct", c, 6) == 0) {
        return InstructionStarter_Struct;
    }
    if (size == 8 && memcmp("Resource", c, 8) == 0) {
        return InstructionStarter_Resource;
    }
    if (size == 7 && memcmp("Variant", c, 7) == 0) {
        return InstructionStarter_Variant;
    }
    if (size == 4 && memcmp("Enum", c, 4) == 0) {
        return InstructionStarter_Enum;
    }
    if (size == 4 && memcmp("Flag", c, 4) == 0) {
        return InstructionStarter_Flag;
    }
    return InstructionStarter_Invalid;
}
static inline InstructionStarter
InstructionStarterFromCaseInsensitiveString(const char* original,
                                            const size_t size)
{
    if (size > InstructionStarterLongestString) {
        return InstructionStarter_Invalid;
    }
    char c[InstructionStarterLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 3 && memcmp("let", c, 3) == 0) {
        return InstructionStarter_Let;
    }
    if (size == 4 && memcmp("mlet", c, 4) == 0) {
        return InstructionStarter_MLet;
    }
    if (size == 10 && memcmp("mutablelet", c, 10) == 0) {
        return InstructionStarter_MutableLet;
    }
    if (size == 8 && memcmp("constant", c, 8) == 0) {
        return InstructionStarter_Constant;
    }
    if (size == 5 && memcmp("const", c, 5) == 0) {
        return InstructionStarter_Const;
    }
    if (size == 3 && memcmp("set", c, 3) == 0) {
        return InstructionStarter_Set;
    }
    if (size == 8 && memcmp("ifreturn", c, 8) == 0) {
        return InstructionStarter_IfReturn;
    }
    if (size == 8 && memcmp("returnif", c, 8) == 0) {
        return InstructionStarter_ReturnIf;
    }
    if (size == 6 && memcmp("return", c, 6) == 0) {
        return InstructionStarter_Return;
    }
    if (size == 3 && memcmp("run", c, 3) == 0) {
        return InstructionStarter_Run;
    }
    if (size == 5 && memcmp("print", c, 5) == 0) {
        return InstructionStarter_Print;
    }
    if (size == 5 && memcmp("error", c, 5) == 0) {
        return InstructionStarter_Error;
    }
    if (size == 7 && memcmp("iterate", c, 7) == 0) {
        return InstructionStarter_Iterate;
    }
    if (size == 6 && memcmp("format", c, 6) == 0) {
        return InstructionStarter_Format;
    }
    if (size == 7 && memcmp("nullary", c, 7) == 0) {
        return InstructionStarter_Nullary;
    }
    if (size == 5 && memcmp("unary", c, 5) == 0) {
        return InstructionStarter_Unary;
    }
    if (size == 6 && memcmp("binary", c, 6) == 0) {
        return InstructionStarter_Binary;
    }
    if (size == 9 && memcmp("procedure", c, 9) == 0) {
        return InstructionStarter_Procedure;
    }
    if (size == 5 && memcmp("while", c, 5) == 0) {
        return InstructionStarter_While;
    }
    if (size == 5 && memcmp("until", c, 5) == 0) {
        return InstructionStarter_Until;
    }
    if (size == 5 && memcmp("match", c, 5) == 0) {
        return InstructionStarter_Match;
    }
    if (size == 9 && memcmp("nocompile", c, 9) == 0) {
        return InstructionStarter_NoCompile;
    }
    if (size == 9 && memcmp("nocleanup", c, 9) == 0) {
        return InstructionStarter_NoCleanup;
    }
    if (size == 2 && memcmp("on", c, 2) == 0) {
        return InstructionStarter_On;
    }
    if (size == 3 && memcmp("off", c, 3) == 0) {
        return InstructionStarter_Off;
    }
    if (size == 6 && memcmp("toggle", c, 6) == 0) {
        return InstructionStarter_Toggle;
    }
    if (size == 5 && memcmp("clear", c, 5) == 0) {
        return InstructionStarter_Clear;
    }
    if (size == 3 && memcmp("all", c, 3) == 0) {
        return InstructionStarter_All;
    }
    if (size == 7 && memcmp("toarray", c, 7) == 0) {
        return InstructionStarter_ToArray;
    }
    if (size == 6 && memcmp("tolist", c, 6) == 0) {
        return InstructionStarter_ToList;
    }
    if (size == 6 && memcmp("verify", c, 6) == 0) {
        return InstructionStarter_Verify;
    }
    if (size == 5 && memcmp("floor", c, 5) == 0) {
        return InstructionStarter_Floor;
    }
    if (size == 5 && memcmp("round", c, 5) == 0) {
        return InstructionStarter_Round;
    }
    if (size == 4 && memcmp("ceil", c, 4) == 0) {
        return InstructionStarter_Ceil;
    }
    if (size == 6 && memcmp("append", c, 6) == 0) {
        return InstructionStarter_Append;
    }
    if (size == 6 && memcmp("insert", c, 6) == 0) {
        return InstructionStarter_Insert;
    }
    if (size == 6 && memcmp("remove", c, 6) == 0) {
        return InstructionStarter_Remove;
    }
    if (size == 10 && memcmp("swapremove", c, 10) == 0) {
        return InstructionStarter_SwapRemove;
    }
    if (size == 3 && memcmp("pop", c, 3) == 0) {
        return InstructionStarter_Pop;
    }
    if (size == 5 && memcmp("empty", c, 5) == 0) {
        return InstructionStarter_Empty;
    }
    if (size == 6 && memcmp("inline", c, 6) == 0) {
        return InstructionStarter_Inline;
    }
    if (size == 7 && memcmp("inlinec", c, 7) == 0) {
        return InstructionStarter_InlineC;
    }
    if (size == 15 && memcmp("inlinecfunction", c, 15) == 0) {
        return InstructionStarter_InlineCFunction;
    }
    if (size == 27 && memcmp("inlinecfunctionreturnstruct", c, 27) == 0) {
        return InstructionStarter_InlineCFunctionReturnStruct;
    }
    if (size == 14 && memcmp("inlinecheaders", c, 14) == 0) {
        return InstructionStarter_InlineCHeaders;
    }
    if (size == 11 && memcmp("inlinecfile", c, 11) == 0) {
        return InstructionStarter_InlineCFile;
    }
    if (size == 10 && memcmp("inlinefile", c, 10) == 0) {
        return InstructionStarter_InlineFile;
    }
    if (size == 14 && memcmp("inlinevariable", c, 14) == 0) {
        return InstructionStarter_InlineVariable;
    }
    if (size == 10 && memcmp("inlinedata", c, 10) == 0) {
        return InstructionStarter_InlineData;
    }
    if (size == 10 && memcmp("inlinetext", c, 10) == 0) {
        return InstructionStarter_InlineText;
    }
    if (size == 5 && memcmp("range", c, 5) == 0) {
        return InstructionStarter_Range;
    }
    if (size == 6 && memcmp("struct", c, 6) == 0) {
        return InstructionStarter_Struct;
    }
    if (size == 8 && memcmp("resource", c, 8) == 0) {
        return InstructionStarter_Resource;
    }
    if (size == 7 && memcmp("variant", c, 7) == 0) {
        return InstructionStarter_Variant;
    }
    if (size == 4 && memcmp("enum", c, 4) == 0) {
        return InstructionStarter_Enum;
    }
    if (size == 4 && memcmp("flag", c, 4) == 0) {
        return InstructionStarter_Flag;
    }
    return InstructionStarter_Invalid;
}
static inline const char*
InstructionStarterToString(const InstructionStarter e)
{
    if (e == InstructionStarter_Let) {
        return "Let";
    }
    if (e == InstructionStarter_MLet) {
        return "MLet";
    }
    if (e == InstructionStarter_MutableLet) {
        return "MutableLet";
    }
    if (e == InstructionStarter_Constant) {
        return "Constant";
    }
    if (e == InstructionStarter_Const) {
        return "Const";
    }
    if (e == InstructionStarter_Set) {
        return "Set";
    }
    if (e == InstructionStarter_IfReturn) {
        return "IfReturn";
    }
    if (e == InstructionStarter_ReturnIf) {
        return "ReturnIf";
    }
    if (e == InstructionStarter_Return) {
        return "Return";
    }
    if (e == InstructionStarter_Run) {
        return "Run";
    }
    if (e == InstructionStarter_Print) {
        return "Print";
    }
    if (e == InstructionStarter_Error) {
        return "Error";
    }
    if (e == InstructionStarter_Iterate) {
        return "Iterate";
    }
    if (e == InstructionStarter_Format) {
        return "Format";
    }
    if (e == InstructionStarter_Nullary) {
        return "Nullary";
    }
    if (e == InstructionStarter_Unary) {
        return "Unary";
    }
    if (e == InstructionStarter_Binary) {
        return "Binary";
    }
    if (e == InstructionStarter_Procedure) {
        return "Procedure";
    }
    if (e == InstructionStarter_While) {
        return "While";
    }
    if (e == InstructionStarter_Until) {
        return "Until";
    }
    if (e == InstructionStarter_Match) {
        return "Match";
    }
    if (e == InstructionStarter_NoCompile) {
        return "NoCompile";
    }
    if (e == InstructionStarter_NoCleanup) {
        return "NoCleanup";
    }
    if (e == InstructionStarter_On) {
        return "On";
    }
    if (e == InstructionStarter_Off) {
        return "Off";
    }
    if (e == InstructionStarter_Toggle) {
        return "Toggle";
    }
    if (e == InstructionStarter_Clear) {
        return "Clear";
    }
    if (e == InstructionStarter_All) {
        return "All";
    }
    if (e == InstructionStarter_ToArray) {
        return "ToArray";
    }
    if (e == InstructionStarter_ToList) {
        return "ToList";
    }
    if (e == InstructionStarter_Verify) {
        return "Verify";
    }
    if (e == InstructionStarter_Floor) {
        return "Floor";
    }
    if (e == InstructionStarter_Round) {
        return "Round";
    }
    if (e == InstructionStarter_Ceil) {
        return "Ceil";
    }
    if (e == InstructionStarter_Append) {
        return "Append";
    }
    if (e == InstructionStarter_Insert) {
        return "Insert";
    }
    if (e == InstructionStarter_Remove) {
        return "Remove";
    }
    if (e == InstructionStarter_SwapRemove) {
        return "SwapRemove";
    }
    if (e == InstructionStarter_Pop) {
        return "Pop";
    }
    if (e == InstructionStarter_Empty) {
        return "Empty";
    }
    if (e == InstructionStarter_Inline) {
        return "Inline";
    }
    if (e == InstructionStarter_InlineC) {
        return "InlineC";
    }
    if (e == InstructionStarter_InlineCFunction) {
        return "InlineCFunction";
    }
    if (e == InstructionStarter_InlineCFunctionReturnStruct) {
        return "InlineCFunctionReturnStruct";
    }
    if (e == InstructionStarter_InlineCHeaders) {
        return "InlineCHeaders";
    }
    if (e == InstructionStarter_InlineCFile) {
        return "InlineCFile";
    }
    if (e == InstructionStarter_InlineFile) {
        return "InlineFile";
    }
    if (e == InstructionStarter_InlineVariable) {
        return "InlineVariable";
    }
    if (e == InstructionStarter_InlineData) {
        return "InlineData";
    }
    if (e == InstructionStarter_InlineText) {
        return "InlineText";
    }
    if (e == InstructionStarter_Range) {
        return "Range";
    }
    if (e == InstructionStarter_Struct) {
        return "Struct";
    }
    if (e == InstructionStarter_Resource) {
        return "Resource";
    }
    if (e == InstructionStarter_Variant) {
        return "Variant";
    }
    if (e == InstructionStarter_Enum) {
        return "Enum";
    }
    if (e == InstructionStarter_Flag) {
        return "Flag";
    }
    return "Invalid";
}