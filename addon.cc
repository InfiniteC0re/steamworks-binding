#include <array>
#include <iostream>
#include "C:\Users\InfiniteC0re\AppData\Local\node-gyp\Cache\12.18.0\include\node\node.h"
#include ".\headers\steam_api.h"

using namespace v8;
using namespace node;

Local<Array> GetAvatar(v8::Isolate* isolate, int avatarHandle) {
    unsigned int avatarHeight;
    unsigned int avatarWidth;

    auto steamUtils = SteamUtils();
    steamUtils->GetImageSize(avatarHandle, &avatarWidth, &avatarHeight);

    const int bufferSize = avatarWidth * avatarHeight * 4;

    uint8 *avatar= new uint8[bufferSize];

    bool gotAvatar = steamUtils->GetImageRGBA(avatarHandle, avatar, bufferSize);
    Local<Array> avatarArray = Array::New(isolate);

    if(gotAvatar) {
        for(int i = 0; i < avatarWidth * avatarHeight * 4; i++) {
            avatarArray->Set(i, Number::New(isolate, avatar[i]));
        }
    }

    return avatarArray;
}

void GYPSteamAPI_Init(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();

    bool state = SteamAPI_Init();
    auto result = v8::Boolean::New(isolate, state);
    
    args.GetReturnValue().Set(result);
}

void GYPSteamAPI_GetFriendCount(const v8::FunctionCallbackInfo<v8::Value>& args) {
    ISteamFriends* steamFriends = SteamFriends();

    v8::Isolate* isolate = args.GetIsolate();

    int count = steamFriends->GetFriendCount(EFriendFlags::k_EFriendFlagAll);

    auto result = v8::Number::New(isolate, count);

    args.GetReturnValue().Set(result);
}

void GYPSteamAPI_GetFriendByIndex(const v8::FunctionCallbackInfo<v8::Value>& args) {
    ISteamFriends* steamFriends = SteamFriends();

    v8::Isolate* isolate = args.GetIsolate();
    auto context = NewContext(isolate);
    
    CSteamID id = steamFriends->GetFriendByIndex(args[0]->Int32Value(context).FromJust(), EFriendFlags::k_EFriendFlagAll);
    auto friendName = v8::String::NewFromUtf8(isolate, steamFriends->GetFriendPersonaName(id));
    auto friendRPC = v8::String::NewFromUtf8(isolate, steamFriends->GetFriendRichPresence(id, "hlsr"));
    auto friendID = v8::Number::New(isolate, (int32)id.GetAccountID());
    int avatarHandle = steamFriends->GetSmallFriendAvatar(id);

    auto avatar = GetAvatar(isolate, avatarHandle);

    Local<Object> nodes = Object::New(isolate);
    nodes->Set(v8::String::NewFromUtf8(isolate, "personaName"), friendName);
    nodes->Set(v8::String::NewFromUtf8(isolate, "friendRPC"), friendRPC);
    nodes->Set(v8::String::NewFromUtf8(isolate, "friendID"), friendID);   
    nodes->Set(v8::String::NewFromUtf8(isolate, "avatar"), avatar);

    args.GetReturnValue().Set(nodes);
}

void GYPSteamAPI_GetFriends(const v8::FunctionCallbackInfo<v8::Value>& args) {
    ISteamFriends* steamFriends = SteamFriends();

    v8::Isolate* isolate = args.GetIsolate();
    auto context = NewContext(isolate);

    Local<String> keyStr = args[0].As<String>();
    
    int count = steamFriends->GetFriendCount(EFriendFlags::k_EFriendFlagAll);
    Local<Array> arr = Array::New(isolate);

    for(int i = 0; i < count; i++) {
        CSteamID id = steamFriends->GetFriendByIndex(i, EFriendFlags::k_EFriendFlagAll);
        auto friendName = v8::String::NewFromUtf8(isolate, steamFriends->GetFriendPersonaName(id));
        auto friendRPC = v8::String::NewFromUtf8(isolate, steamFriends->GetFriendRichPresence(id, (const char*)*keyStr));
        auto friendID = v8::Number::New(isolate, (int32)id.GetAccountID());
        int avatarHandle = steamFriends->GetSmallFriendAvatar(id);

        auto avatar = GetAvatar(isolate, avatarHandle);

        Local<Object> obj = Object::New(isolate);
        obj->Set(v8::String::NewFromUtf8(isolate, "personaName"), friendName);
        obj->Set(v8::String::NewFromUtf8(isolate, "friendRPC"), friendRPC);
        obj->Set(v8::String::NewFromUtf8(isolate, "friendID"), friendID);
        obj->Set(v8::String::NewFromUtf8(isolate, "avatar"), avatar);

        arr->Set(i, obj);
    }

    args.GetReturnValue().Set(arr);
}

void GYPSteamAPI_SetRichPresense(const v8::FunctionCallbackInfo<v8::Value>& args) {
    ISteamFriends* steamFriends = SteamFriends();

    v8::Isolate* isolate = args.GetIsolate();
    auto context = NewContext(isolate);

    Local<String> keyStr = args[0].As<String>();
    Local<String> valStr = args[1].As<String>();

    const char* key = (const char*)*keyStr;
    const char* val = (const char*)*valStr;

    auto result = v8::Boolean::New(isolate, steamFriends->SetRichPresence(key, val));
    args.GetReturnValue().Set(result);

}

void GYPSteamAPI_RunCallbacks(const v8::FunctionCallbackInfo<v8::Value>& args) {
    SteamAPI_RunCallbacks();
}

void Initialize(v8::Local<v8::Object> exports) {
    NODE_SET_METHOD(exports, "SteamAPI_Init", GYPSteamAPI_Init);
    NODE_SET_METHOD(exports, "SteamAPI_RunCallbacks", GYPSteamAPI_RunCallbacks);
    NODE_SET_METHOD(exports, "GetFriendCount", GYPSteamAPI_GetFriendCount);
    NODE_SET_METHOD(exports, "GetFriendByIndex", GYPSteamAPI_GetFriendByIndex);
    NODE_SET_METHOD(exports, "SetRichPresense", GYPSteamAPI_SetRichPresense);
    NODE_SET_METHOD(exports, "GetFriends", GYPSteamAPI_GetFriends);
}

NODE_MODULE(addon, Initialize)