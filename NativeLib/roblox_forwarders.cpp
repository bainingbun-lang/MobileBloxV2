// roblox_forwarders.cpp
// Forwarders to Roblox's internal Luau functions
// Replaces the modified Luau source files approach

#include <lstate.h>
#include <lobject.h>
#include <ltable.h>
#include <ldo.h>
#include <lgc.h>
#include <lvmexecute.h>
#include <lvmutils.h>

#include "utils/utils.hpp"
#include "roblox/update.hpp"

using namespace roblox::addresses;

// =============================================================================
// LUAVM FORWARDERS - Call into Roblox's libroblox.so Luau implementation
// =============================================================================

// luaD_call -> luau_execute
extern "C" void luau_execute(lua_State* L) {
    static auto func = *reinterpret_cast<decltype(luau_execute)*>(
        utils::memory::rebase("libroblox.so", luau_execute)
    );
    func(L);
}

// luaD_throw
extern "C" void luaD_throw(lua_State* L, int errcode) {
    static auto func = *reinterpret_cast<decltype(luaD_throw)*>(
        utils::memory::rebase("libroblox.so", luaD_throw)
    );
    func(L, errcode);
}

// luaD_rawrunprotected
extern "C" int luaD_rawrunprotected(lua_State* L, Pfunc f, void* ud) {
    static auto func = *reinterpret_cast<decltype(luaD_rawrunprotected)*>(
        utils::memory::rebase("libroblox.so", luaD_rawrunprotected)
    );
    return func(L, f, ud);
}

// luaC_step
extern "C" size_t luaC_step(lua_State* L, bool assist) {
    static auto func = *reinterpret_cast<decltype(luaC_step)*>(
        utils::memory::rebase("libroblox.so", luaC_step)
    );
    return func(L, assist);
}

// luaC_barriertable
extern "C" void luaC_barriertable(lua_State* L, Table* t, GCObject* v) {
    static auto func = *reinterpret_cast<decltype(luaC_barriertable)*>(
        utils::memory::rebase("libroblox.so", luaC_barriertable)
    );
    func(L, t, v);
}

// luaV_gettable
extern "C" void luaV_gettable(lua_State* L, const TValue* t, TValue* key, StkId val) {
    static auto func = *reinterpret_cast<decltype(luaV_gettable)*>(
        utils::memory::rebase("libroblox.so", luaV_gettable)
    );
    func(L, t, key, val);
}

// luaV_settable
extern "C" void luaV_settable(lua_State* L, const TValue* t, TValue* key, StkId val) {
    static auto func = *reinterpret_cast<decltype(luaV_settable)*>(
        utils::memory::rebase("libroblox.so", luaV_settable)
    );
    func(L, t, key, val);
}

// =============================================================================
// GLOBAL OBJECTS - Rebased to Roblox's libroblox.so
// =============================================================================

// luaO_nilobject - defined in luaconf.h as macro, but we need the actual address
extern "C" TValue* luaO_nilobject_ptr() {
    return reinterpret_cast<TValue*>(utils::memory::rebase("libroblox.so", luaO_nilobj));
}

// dummynode - for luaH_getnum
extern "C" LuaNode* luaH_dummynode_ptr() {
    return reinterpret_cast<LuaNode*>(utils::memory::rebase("libroblox.so", dummynode));
}

// =============================================================================
// INITIALIZATION - Call once at startup to verify addresses
// =============================================================================
namespace roblox::forwarders {
    bool initialize() {
        // Verify all function pointers resolve
        void* checks[] = {
            (void*)utils::memory::rebase("libroblox.so", luau_execute),
            (void*)utils::memory::rebase("libroblox.so", luaD_throw),
            (void*)utils::memory::rebase("libroblox.so", luaD_rawrunprotected),
            (void*)utils::memory::rebase("libroblox.so", luaC_step),
            (void*)utils::memory::rebase("libroblox.so", luaC_barriertable),
            (void*)utils::memory::rebase("libroblox.so", luaV_gettable),
            (void*)utils::memory::rebase("libroblox.so", luaV_settable),
            (void*)utils::memory::rebase("libroblox.so", luaO_nilobj),
            (void*)utils::memory::rebase("libroblox.so", dummynode),
        };

        for (void* ptr : checks) {
            if (!ptr || ptr == (void*)0x1) {
                LOGE("[forwarders] Failed to resolve address: %p", ptr);
                return false;
            }
        }

        LOGI("[forwarders] All Roblox Luau forwarders initialized successfully");
        return true;
    }
}