#include <jni.h>

#include <cstdlib>
#include <cstring>
#include <string>

#include "luacode.h"
#include "lua.h"
#include "lualib.h"

namespace {

struct ExecutionBudget {
    long long remaining;
};

void interrupt(lua_State* L, int gc) {
    if (gc >= 0) {
        return;
    }
    auto* budget = static_cast<ExecutionBudget*>(lua_callbacks(L)->userdata);
    if (budget != nullptr && --budget->remaining <= 0) {
        luaL_error(L, "script exceeded the execution limit");
    }
}

void appendOutput(lua_State* L, const std::string& text) {
    lua_getglobal(L, "__output");
    const char* previous = lua_tostring(L, -1);
    std::string combined = previous != nullptr ? previous : "";
    combined += text;
    lua_pop(L, 1);
    lua_pushlstring(L, combined.data(), combined.size());
    lua_setglobal(L, "__output");
}

int l_print(lua_State* L) {
    const int count = lua_gettop(L);
    std::string line;
    for (int index = 1; index <= count; ++index) {
        size_t length = 0;
        const char* value = luaL_tolstring(L, index, &length);
        if (index > 1) {
            line += "\t";
        }
        if (value != nullptr) {
            line.append(value, length);
        }
        lua_pop(L, 1);
    }
    line += "\n";
    appendOutput(L, line);
    return 0;
}

int l_loadstring(lua_State* L) {
    size_t length = 0;
    const char* source = luaL_checklstring(L, 1, &length);
    lua_CompileOptions options{};
    options.optimizationLevel = 1;
    options.debugLevel = 1;
    size_t size = 0;
    char* bytecode = luau_compile(source, length, &options, &size);
    if (bytecode == nullptr) {
        lua_pushnil(L);
        lua_pushstring(L, "compile failed");
        return 2;
    }
    int status = luau_load(L, "=loadstring", bytecode, size, 0);
    free(bytecode);
    if (status != 0) {
        lua_pushnil(L);
        lua_insert(L, -2);
        return 2;
    }
    return 1;
}

lua_State* createState() {
    lua_State* L = luaL_newstate();
    if (L == nullptr) {
        return nullptr;
    }
    luaL_openlibs(L);
    lua_pushcfunction(L, l_print, "print");
    lua_setglobal(L, "print");
    lua_pushcfunction(L, l_loadstring, "loadstring");
    lua_setglobal(L, "loadstring");
    lua_pushstring(L, "");
    lua_setglobal(L, "__output");

    lua_Callbacks* callbacks = lua_callbacks(L);
    callbacks->userdata = new ExecutionBudget{50000000LL};
    callbacks->interrupt = interrupt;
    return L;
}

std::string finish(lua_State* L, int status) {
    std::string result;
    if (status != 0) {
        const char* error = lua_tostring(L, -1);
        result += "error: ";
        result += error != nullptr ? error : "unknown error";
        result += "\n";
        lua_pop(L, 1);
    }
    lua_getglobal(L, "__output");
    const char* output = lua_tostring(L, -1);
    if (output != nullptr) {
        result += output;
    }
    auto* budget = static_cast<ExecutionBudget*>(lua_callbacks(L)->userdata);
    lua_close(L);
    delete budget;
    return result;
}

char* compileSource(JNIEnv* env, jstring source, size_t* size) {
    const char* text = env->GetStringUTFChars(source, nullptr);
    if (text == nullptr) {
        return nullptr;
    }
    lua_CompileOptions options{};
    options.optimizationLevel = 1;
    options.debugLevel = 1;
    char* bytecode = luau_compile(text, strlen(text), &options, size);
    env->ReleaseStringUTFChars(source, text);
    return bytecode;
}

}  // namespace

extern "C" JNIEXPORT jbyteArray JNICALL
Java_com_mobileblox_v2_luau_LuauBridge_nativeCompile(JNIEnv* env, jclass, jstring source) {
    if (source == nullptr) {
        return nullptr;
    }
    size_t size = 0;
    char* bytecode = compileSource(env, source, &size);
    if (bytecode == nullptr) {
        return nullptr;
    }
    jbyteArray result = env->NewByteArray(static_cast<jsize>(size));
    if (result != nullptr) {
        env->SetByteArrayRegion(
            result,
            0,
            static_cast<jsize>(size),
            reinterpret_cast<const jbyte*>(bytecode));
    }
    free(bytecode);
    return result;
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_mobileblox_v2_luau_LuauBridge_nativeRun(JNIEnv* env, jclass, jstring source) {
    if (source == nullptr) {
        return env->NewStringUTF("error: missing source");
    }
    size_t size = 0;
    char* bytecode = compileSource(env, source, &size);
    if (bytecode == nullptr) {
        return env->NewStringUTF("error: compile failed");
    }

    lua_State* L = createState();
    if (L == nullptr) {
        free(bytecode);
        return env->NewStringUTF("error: cannot create Luau state");
    }
    int status = luau_load(L, "=script", bytecode, size, 0);
    free(bytecode);
    if (status == 0) {
        status = lua_pcall(L, 0, 0, 0);
    }
    const std::string result = finish(L, status);
    return env->NewStringUTF(result.c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_mobileblox_v2_luau_LuauBridge_nativeRunBytecode(
    JNIEnv* env,
    jclass,
    jbyteArray bytecode) {
    if (bytecode == nullptr) {
        return env->NewStringUTF("error: missing bytecode");
    }
    const jsize size = env->GetArrayLength(bytecode);
    jbyte* data = env->GetByteArrayElements(bytecode, nullptr);
    if (data == nullptr) {
        return env->NewStringUTF("error: cannot read bytecode");
    }

    lua_State* L = createState();
    if (L == nullptr) {
        env->ReleaseByteArrayElements(bytecode, data, JNI_ABORT);
        return env->NewStringUTF("error: cannot create Luau state");
    }
    int status = luau_load(
        L,
        "=bytecode",
        reinterpret_cast<const char*>(data),
        static_cast<size_t>(size),
        0);
    env->ReleaseByteArrayElements(bytecode, data, JNI_ABORT);
    if (status == 0) {
        status = lua_pcall(L, 0, 0, 0);
    }
    const std::string result = finish(L, status);
    return env->NewStringUTF(result.c_str());
}
