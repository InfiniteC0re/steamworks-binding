#include <array>
#include <string>
#include <node.h>
#include ".\headers\steam_api.h"

using namespace v8;
using namespace node;

Local<Array> GetAvatar(Isolate* isolate, int avatarHandle) {
    unsigned int avatarHeight;
    unsigned int avatarWidth;

    auto steamUtils = SteamUtils();
    steamUtils->GetImageSize(avatarHandle, &avatarWidth, &avatarHeight);

    const int bufferSize = avatarWidth * avatarHeight * 4;
    auto context = isolate->GetCurrentContext();

    uint8 *avatar= new uint8[bufferSize];

    bool gotAvatar = steamUtils->GetImageRGBA(avatarHandle, avatar, bufferSize);
    Local<Array> avatarArray = Array::New(isolate);

    if(gotAvatar) {
        for(int i = 0; i < avatarWidth * avatarHeight * 4; i++) {
            avatarArray->Set(context, i, Number::New(isolate, avatar[i]));
        }
    }

    return avatarArray;
}

Local<Object> GetFriendInfo(Isolate* isolate, ISteamFriends* steamFriends, CSteamID id, const char* keyStr, bool getAvatars) {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Value> personaName = String::NewFromUtf8(isolate, steamFriends->GetFriendPersonaName(id)).ToLocalChecked();
    Local<Value> personaState = Number::New(isolate, steamFriends->GetFriendPersonaState(id));
    Local<Value> friendRPC = String::NewFromUtf8(isolate, steamFriends->GetFriendRichPresence(id, keyStr)).ToLocalChecked();
    uint64_t id64 = id.ConvertToUint64();
    std::string id64str = std::to_string(id64);

    Local<Value> friendID = String::NewFromUtf8(isolate, id64str.c_str()).ToLocalChecked();

    FriendGameInfo_t gameInfo;
    bool playingGame = steamFriends->GetFriendGamePlayed(id, &gameInfo);

    Local<Object> nodes = Object::New(isolate);
    nodes->Set(context, String::NewFromUtf8(isolate, "personaName").ToLocalChecked(), personaName);
    nodes->Set(context, String::NewFromUtf8(isolate, "personaState").ToLocalChecked(), personaState);
    nodes->Set(context, String::NewFromUtf8(isolate, "friendRPC").ToLocalChecked(), friendRPC);
    nodes->Set(context, String::NewFromUtf8(isolate, "friendID").ToLocalChecked(), friendID);
    nodes->Set(context, String::NewFromUtf8(isolate, "gamePlayed").ToLocalChecked(), Boolean::New(isolate, playingGame));

    if(playingGame) {
        nodes->Set(context, String::NewFromUtf8(isolate, "appID").ToLocalChecked(), Number::New(isolate, gameInfo.m_gameID.AppID()));
    }
    
    if(getAvatars) {
        int avatarHandle = steamFriends->GetMediumFriendAvatar(id);
        Local<Array> avatar = GetAvatar(isolate, avatarHandle);
        nodes->Set(context, String::NewFromUtf8(isolate, "avatar").ToLocalChecked(), avatar);
    }

    return nodes;
}

void GYPSteamAPI_GetPersonaStates(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    
    Local<Array> result;

    result->Set(context, Number::New(isolate, EPersonaState::k_EPersonaStateAway), String::NewFromUtf8(isolate, "k_EPersonaStateAway").ToLocalChecked());
    result->Set(context, Number::New(isolate, EPersonaState::k_EPersonaStateBusy), String::NewFromUtf8(isolate, "k_EPersonaStateBusy").ToLocalChecked());
    result->Set(context, Number::New(isolate, EPersonaState::k_EPersonaStateInvisible), String::NewFromUtf8(isolate, "k_EPersonaStateInvisible").ToLocalChecked());
    result->Set(context, Number::New(isolate, EPersonaState::k_EPersonaStateLookingToPlay), String::NewFromUtf8(isolate, "k_EPersonaStateLookingToPlay").ToLocalChecked());
    result->Set(context, Number::New(isolate, EPersonaState::k_EPersonaStateLookingToTrade), String::NewFromUtf8(isolate, "k_EPersonaStateLookingToTrade").ToLocalChecked());
    result->Set(context, Number::New(isolate, EPersonaState::k_EPersonaStateMax), String::NewFromUtf8(isolate, "k_EPersonaStateMax").ToLocalChecked());
    result->Set(context, Number::New(isolate, EPersonaState::k_EPersonaStateOffline), String::NewFromUtf8(isolate, "k_EPersonaStateOffline").ToLocalChecked());
    result->Set(context, Number::New(isolate, EPersonaState::k_EPersonaStateOnline), String::NewFromUtf8(isolate, "k_EPersonaStateOnline").ToLocalChecked());
    result->Set(context, Number::New(isolate, EPersonaState::k_EPersonaStateSnooze), String::NewFromUtf8(isolate, "k_EPersonaStateSnooze").ToLocalChecked());

    args.GetReturnValue().Set(result);
}

void GYPSteamAPI_Init(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    bool state = SteamAPI_Init();
    Local<Boolean> result = Boolean::New(isolate, state);
    
    args.GetReturnValue().Set(result);
}

void GYPSteamAPI_GetPersonName(const FunctionCallbackInfo<Value>& args) {
    ISteamFriends* steamFriends = SteamFriends();

    Isolate* isolate = args.GetIsolate();

    Local<Value> personaName = String::NewFromUtf8(isolate, steamFriends->GetPersonaName()).ToLocalChecked();

    args.GetReturnValue().Set(personaName);
}

void GYPSteamAPI_GetFriendCount(const FunctionCallbackInfo<Value>& args) {
    ISteamFriends* steamFriends = SteamFriends();

    Isolate* isolate = args.GetIsolate();

    int count = steamFriends->GetFriendCount(EFriendFlags::k_EFriendFlagAll);

    Local<Number> result = Number::New(isolate, count);

    args.GetReturnValue().Set(result);
}

void GYPSteamAPI_GetFriendByIndex(const FunctionCallbackInfo<Value>& args) {
    ISteamFriends* steamFriends = SteamFriends();

    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    v8::String::Utf8Value keyStr(isolate, args[1]);
    bool getAvatars = args[2]->BooleanValue(isolate);
    
    CSteamID id = steamFriends->GetFriendByIndex(args[0]->Int32Value(context).FromJust(), EFriendFlags::k_EFriendFlagAll);

    Local<Object> nodes = GetFriendInfo(isolate, steamFriends, id, (const char*)*keyStr, getAvatars);

    args.GetReturnValue().Set(nodes);
}

void GYPSteamAPI_GetFriends(const FunctionCallbackInfo<Value>& args) {
    ISteamFriends* steamFriends = SteamFriends();

    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    v8::String::Utf8Value keyStr(isolate, args[0]);
    bool getAvatars = args[1]->BooleanValue(isolate);
    
    int count = steamFriends->GetFriendCount(EFriendFlags::k_EFriendFlagAll);
    Local<Array> arr = Array::New(isolate);

    for(int i = 0; i < count; i++) {
        CSteamID id = steamFriends->GetFriendByIndex(i, EFriendFlags::k_EFriendFlagAll);

        Local<Object> obj = GetFriendInfo(isolate, steamFriends, id, (const char*)*keyStr, getAvatars);

        arr->Set(context, i, obj);
    }

    args.GetReturnValue().Set(arr);
}

void GYPSteamAPI_SetRichPresense(const FunctionCallbackInfo<Value>& args) {
    ISteamFriends* steamFriends = SteamFriends();

    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();

    v8::String::Utf8Value keyStr(isolate, args[0]);
    v8::String::Utf8Value valStr(isolate, args[1]);

    const char* key = (const char*)*keyStr;
    const char* val = (const char*)*valStr;

    Local<Boolean> result = Boolean::New(isolate, steamFriends->SetRichPresence(key, val));
    args.GetReturnValue().Set(result);
}

void GYPSteamAPI_RunCallbacks(const FunctionCallbackInfo<Value>& args) {
    SteamAPI_RunCallbacks();
}

void Initialize(Local<Object> exports) {
    NODE_SET_METHOD(exports, "SteamAPI_Init", GYPSteamAPI_Init);
    NODE_SET_METHOD(exports, "SteamAPI_RunCallbacks", GYPSteamAPI_RunCallbacks);
    NODE_SET_METHOD(exports, "GetFriendCount", GYPSteamAPI_GetFriendCount);
    NODE_SET_METHOD(exports, "GetPersonaStates", GYPSteamAPI_GetPersonaStates);
    NODE_SET_METHOD(exports, "GetFriendByIndex", GYPSteamAPI_GetFriendByIndex);
    NODE_SET_METHOD(exports, "SetRichPresense", GYPSteamAPI_SetRichPresense);
    NODE_SET_METHOD(exports, "GetFriends", GYPSteamAPI_GetFriends);
    NODE_SET_METHOD(exports, "GetPersonName", GYPSteamAPI_GetPersonName);
}

NODE_MODULE(addon, Initialize)