#!/usr/bin/python3

from concurrent.futures import ThreadPoolExecutor

from makeEnum import makeEnum

futures = []
with ThreadPoolExecutor(max_workers=8) as e:

    futures.append(
        e.submit(makeEnum, 'NumberType', ['Signed', 'Unsigned', 'Float']))

    futures.append(e.submit(makeEnum, 'ValueType', [
        'Null', 'Number', 'Boolean', 'Type', 'String', 'Flag',
        'Enum', 'Struct', 'Variant', 'List', 'External', 'Data']))

    futures.append(e.submit(makeEnum, 'TokenType', [
        'Keyword', 'InstructionStarter', 'GetOperator',
        'NumberOperator', 'BooleanOperator', 'ComparisonOperator',
        'String', 'Character', 'Number', 'Type', 'Identifier',
        'FunctionCall',  'ListInitialization',
        'Match', 'Iterate', 'NumberRound',
        'Expression', 'List', 'Array',  'Scope', 'Struct']))

    futures.append(e.submit(makeEnum, 'Keyword', [
        'Null', 'True', 'False', 'external', 'bool', 'string',
        'i8', 'i16', 'i32', 'i64',
        'u8', 'u16', 'u32', 'u64',
        'f32', 'f64']))

    futures.append(e.submit(makeEnum, 'FunctionType',
                   ['Nullary', 'Unary', 'Binary', 'Procedure']))

    futures.append(e.submit(makeEnum, 'NumberOperator', [
        'Add', 'Subtract', 'Multiply', 'Divide', 'Modulo']))

    futures.append(
        e.submit(makeEnum, 'BooleanOperator', ['And', 'Or', 'Xor', 'Not']))

    futures.append(e.submit(makeEnum, 'ComparisonOperator', [
        'LessThan',  'EqualTo', 'GreaterThan']))

    futures.append(e.submit(makeEnum, 'OperatorType', [
        'Get', 'Number', 'Comparison', 'Function',  'Boolean']))

    futures.append(e.submit(makeEnum, 'GetOperator',
                   ['Member', 'Type', 'MakeList', 'Length']))

    futures.append(e.submit(makeEnum, 'InstructionType', [
        'Return', 'IfReturn', 'While', 'Until', 'Iterate',
        'Run', 'Print', 'Error', 'Match', 'Format', 'Verify',
        'CreateVariable', 'UpdateVariable', 'ConvertContainer',
        'Inline', 'InlineC', 'InlineCFunction', 'InlineCFunctionReturnStruct',
        'InlineCHeaders', 'InlineCFile', 'InlineFile', 'InlineVariable',
        'DefineRange', 'DefineStruct', 'DefineEnum',  'DefineFunction',
        'InlineData', 'InlineText', 'NoCompile', 'NoCleanup',
        'ChangeFlag', 'SetAllFlag', 'ListModify', 'NumberRound']))

    futures.append(e.submit(makeEnum, 'InstructionStarter', [
        'Let', 'MLet', 'MutableLet', 'Constant', 'Const', 'Set',
        'IfReturn', 'ReturnIf', 'Return', 'Run',
        'Print', 'Error', 'Iterate', 'Format',
        'Nullary', 'Unary', 'Binary', 'Procedure',
        'While', 'Until', 'Match', 'NoCompile', 'NoCleanup',
        'On', 'Off', 'Toggle', 'Clear', 'All',
        'ToArray', 'ToList', 'Verify',
        'Floor', 'Round', 'Ceil',
        'Append', 'Insert', 'Remove', 'SwapRemove', 'Pop', 'Empty',
        'Inline', 'InlineC', 'InlineCFunction', 'InlineCFunctionReturnStruct',
        'InlineCHeaders', 'InlineCFile', 'InlineFile', 'InlineVariable',
        'InlineData', 'InlineText',
        'Range', 'Struct', 'Resource', 'Variant', 'Enum', 'Flag']))

    futures.append(e.submit(makeEnum, 'NumberRound',
                   ['Floor', 'Round', 'Ceil']))

    futures.append(e.submit(makeEnum, 'ListModifyType', [
                   'Append', 'Insert', 'Remove', 'SwapRemove', 'Pop', 'Empty']))

    futures.append(e.submit(makeEnum, 'AtomType', [
        'Variable', 'Function', 'Enum', 'Range', 'Struct']))

    futures.append(e.submit(makeEnum, 'ExpressionType', [
        'Nullary', 'UnaryValue', 'UnaryVariable', 'UnaryScope', 'UnaryStruct',
        'UnaryList', 'UnaryMatch', 'Binary']))

    futures.append(e.submit(makeEnum, 'VariableType', [
                   'Immutable', 'Mutable', 'Constant']))

    futures.append(e.submit(makeEnum, 'StateCommand', ['Print']))

    futures.append(e.submit(makeEnum, 'ChangeFlagType', [
        'Add', 'Remove', 'Toggle']))

    futures.append(e.submit(makeEnum, 'MatchBranchType',
                   ['None', 'Expression', 'Instructions']))

    futures.append(e.submit(makeEnum, 'CType', [
        'u8', 'u16', 'u32', 'u64',
        'i8', 'i16', 'i32', 'i64',
        'f32', 'f64',
        'bool', 'string', 'ptr']))
    for f in futures:
        x = f.result()
        if x:
            print(x)
