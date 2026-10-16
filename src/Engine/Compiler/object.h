#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <cstdint>
#include <functional>

#include "Memory/elementtype.h"
#include "value_nanbox.h"

#ifndef IN_PGCOMPILER_TESTS_BUILD
// Debugging features
// Flag used to trace execution step-by-step (With the stack contents)
// #define DEBUG_TRACE_EXECUTION

// Flag to debug runtime memory management (tracking allocations/frees)
// #define DEBUG_RUNTIME_MEMORY

// Flag to unforce stack checks that should never fail
// #define DEBUG_CHECK_STACK

// Flag to profile compile times
// #define DEBUG_PROFILE_COMPILE

// Flag to print tokens during lexing
// #define DEBUG_PRINT_TOKENS

// Flag to print compiled bytecode
// #define DEBUG_PRINT_CODE
#endif

namespace pg
{
    // Forward declarations
    struct ObjFunction;
    struct ObjUpvalue;
    struct Klass;
    struct ObjInstance;
    struct ObjBoundMethod;
    struct ComponentProxyMetadata;
    struct VM;

    // Value is now defined in value_nanbox.h as uint64_t
    typedef uint64_t Value;

    typedef std::function<Value(VM* vm, int argCount, Value* args)> NativeFn;

    struct NativeFunction
    {
        NativeFn function;
    };

    struct Closure
    {
        Closure(ObjFunction* func);

        ObjFunction* function;
        std::vector<ObjUpvalue*> upvalues;
    };

    enum class FunctionType
    {
        TYPE_FUNCTION,
        TYPE_METHOD,
        TYPE_INITIALIZER,
        TYPE_SCRIPT,
    };

    enum class InterpretResult
    {
        OK,
        COMPILE_ERROR,
        RUNTIME_ERROR
    };

    // Value type checking and extraction macros are now in value_nanbox.h

    struct DecodedInstruction;

    struct CallFrame
    {
        Closure *closure;
        uint8_t *ip;
        Value *slots;
        Value *stackBase;  // Where the caller's stack ends (position to truncate to on return)

        // Decoded instruction in the caller's decoded chunk to resume at
        // when this frame returns. Set at frame-push time by VM::call /
        // VM::callBound from vm->pendingCallResume (written by the frame-
        // pushing decoded handler). Returned by op_return_decoded straight
        // to the dispatch loop.
        const DecodedInstruction* callerResume = nullptr;
    };

    // Value creation functions are now in value_nanbox.h
    // Convenience macros for backward compatibility
    #define BOOL_VAL(value)           makeBoolValue(value)
    #define INT_VAL(value)            makeIntValue(value)
    #define FLOAT_VAL(value)          makeFloatValue(value)
    #define DOUBLE_VAL(value)         makeDoubleValue(value)

    struct ObjUpvalue
    {
        ObjUpvalue(Value* slot) : location(slot) {}

        Value* location;
        Value closed = INT_VAL(0); // Closed-over value when moved from stack to heap
        ObjUpvalue* next = nullptr;
    };

    struct Klass
    {
        Klass(const std::string& className) : name(className) {}

        std::string name;
        std::unordered_map<std::string, Value> methods;
    };

    struct ObjInstance
    {
        ObjInstance(Klass* klass) : klass(klass) {}

        Klass* klass;

        // Only set on component proxies (see componentproxy.h): the C++ component the instance forwards to and its property table
        void* proxyTarget = nullptr;

        const ComponentProxyMetadata* proxyMeta = nullptr;

        // ---- The fields ----------------------------------------------------------------------
        // In the order they were added: a slot is a field's place in that order, and stays its
        // field's (a field is never removed). Everything outside goes through these, never the
        // storage, so that the storage can change in one place.

        static constexpr size_t npos = static_cast<size_t>(-1);

        inline size_t fieldCount() const { return fieldValueList.size(); }

        inline const std::string& fieldName(size_t slot) const { return fieldNameList[slot]; }

        inline Value fieldValue(size_t slot) const { return fieldValueList[slot]; }

        // The value in place, to replace it (the caller releases and retains)
        inline Value& fieldValueRef(size_t slot) { return fieldValueList[slot]; }

        // Every value, in slot order (releasing them all, handing them to C++ as a list)
        inline std::vector<Value>& allFieldValues() { return fieldValueList; }

        // The slot of a field, npos when the instance has none
        inline size_t findField(const std::string& name) const
        {
            auto it = fieldIndex.find(name);

            return it == fieldIndex.end() ? npos : it->second;
        }

        // Helper to get a field value (none if not found)
        inline Value getField(const std::string& name) const
        {
            const size_t slot = findField(name);

            return slot == npos ? makeNoneValue() : fieldValueList[slot];
        }

        inline bool hasField(const std::string& name) const
        {
            return findField(name) != npos;
        }

        // Helper to set a field value (creates or updates)
        void setField(const std::string& name, Value value, VM *vm = nullptr, bool deleteOld = false);

        // A field added after the others: its slot. A name already there gets a second slot, and is
        // found at the new one from then on (what a table literal writing a key twice does)
        inline size_t appendField(std::string name, Value value)
        {
            const size_t slot = fieldValueList.size();
            fieldValueList.push_back(value);
            fieldIndex[name] = slot;
            fieldNameList.push_back(std::move(name));

            return slot;
        }

        inline void reserveFields(size_t count)
        {
            fieldValueList.reserve(count);
            fieldNameList.reserve(count);
            fieldIndex.reserve(count);
        }

    private:
        // Values in slot order, a name a slot beside them, and the name -> slot index
        std::vector<Value> fieldValueList;
        std::vector<std::string> fieldNameList;
        std::unordered_map<std::string, size_t> fieldIndex;
    };

    struct ObjBoundMethod
    {
        ObjBoundMethod(const Value& receiver, Closure* method) : receiver(receiver), method(method) {}

        Value receiver;
        Closure* method;
    };

    struct ObjVector
    {
        ObjVector() {}

        std::vector<Value> fields;
    };

    // Note: elementToValue, valueToElement, copyValue, getValueAsInt are now VM member functions
    // Access them via: vm->elementToValue(), vm->valueToElement(), etc.
}