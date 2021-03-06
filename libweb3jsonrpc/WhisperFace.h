#ifndef JSONRPC_CPP_STUB_DEV_RPC_WHISPERFACE_H_
#define JSONRPC_CPP_STUB_DEV_RPC_WHISPERFACE_H_

#include "ModularServer.h"

namespace dev {
    namespace rpc {
        class WhisperFace : public ServerInterface<WhisperFace>
        {
            public:
                WhisperFace()
                {
                    this->bindAndAddMethod(jsonrpc::Procedure("shh_post", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_BOOLEAN, "param1",jsonrpc::JSON_OBJECT, NULL), &dev::rpc::WhisperFace::shh_postI);
                    this->bindAndAddMethod(jsonrpc::Procedure("shh_newIdentity", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_STRING,  NULL), &dev::rpc::WhisperFace::shh_newIdentityI);
                    this->bindAndAddMethod(jsonrpc::Procedure("shh_hasIdentity", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_BOOLEAN, "param1",jsonrpc::JSON_STRING, NULL), &dev::rpc::WhisperFace::shh_hasIdentityI);
                    this->bindAndAddMethod(jsonrpc::Procedure("shh_newGroup", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_STRING, "param1",jsonrpc::JSON_STRING,"param2",jsonrpc::JSON_STRING, NULL), &dev::rpc::WhisperFace::shh_newGroupI);
                    this->bindAndAddMethod(jsonrpc::Procedure("shh_addToGroup", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_STRING, "param1",jsonrpc::JSON_STRING,"param2",jsonrpc::JSON_STRING, NULL), &dev::rpc::WhisperFace::shh_addToGroupI);
                    this->bindAndAddMethod(jsonrpc::Procedure("shh_newFilter", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_STRING, "param1",jsonrpc::JSON_OBJECT, NULL), &dev::rpc::WhisperFace::shh_newFilterI);
                    this->bindAndAddMethod(jsonrpc::Procedure("shh_uninstallFilter", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_BOOLEAN, "param1",jsonrpc::JSON_STRING, NULL), &dev::rpc::WhisperFace::shh_uninstallFilterI);
                    this->bindAndAddMethod(jsonrpc::Procedure("shh_getFilterChanges", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_ARRAY, "param1",jsonrpc::JSON_STRING, NULL), &dev::rpc::WhisperFace::shh_getFilterChangesI);
                    this->bindAndAddMethod(jsonrpc::Procedure("shh_getMessages", jsonrpc::PARAMS_BY_POSITION, jsonrpc::JSON_ARRAY, "param1",jsonrpc::JSON_STRING, NULL), &dev::rpc::WhisperFace::shh_getMessagesI);
                }

                inline virtual void shh_postI(const Json::Value &request, Json::Value &response)
                {
                    response = this->shh_post(request[0u]);
                }
                inline virtual void shh_newIdentityI(const Json::Value &request, Json::Value &response)
                {
                    (void)request;
                    response = this->shh_newIdentity();
                }
                inline virtual void shh_hasIdentityI(const Json::Value &request, Json::Value &response)
                {
                    response = this->shh_hasIdentity(request[0u].asString());
                }
                inline virtual void shh_newGroupI(const Json::Value &request, Json::Value &response)
                {
                    response = this->shh_newGroup(request[0u].asString(), request[1u].asString());
                }
                inline virtual void shh_addToGroupI(const Json::Value &request, Json::Value &response)
                {
                    response = this->shh_addToGroup(request[0u].asString(), request[1u].asString());
                }
                inline virtual void shh_newFilterI(const Json::Value &request, Json::Value &response)
                {
                    response = this->shh_newFilter(request[0u]);
                }
                inline virtual void shh_uninstallFilterI(const Json::Value &request, Json::Value &response)
                {
                    response = this->shh_uninstallFilter(request[0u].asString());
                }
                inline virtual void shh_getFilterChangesI(const Json::Value &request, Json::Value &response)
                {
                    response = this->shh_getFilterChanges(request[0u].asString());
                }
                inline virtual void shh_getMessagesI(const Json::Value &request, Json::Value &response)
                {
                    response = this->shh_getMessages(request[0u].asString());
                }
                virtual bool shh_post(const Json::Value& param1) = 0;
                virtual std::string shh_newIdentity() = 0;
                virtual bool shh_hasIdentity(const std::string& param1) = 0;
                virtual std::string shh_newGroup(const std::string& param1, const std::string& param2) = 0;
                virtual std::string shh_addToGroup(const std::string& param1, const std::string& param2) = 0;
                virtual std::string shh_newFilter(const Json::Value& param1) = 0;
                virtual bool shh_uninstallFilter(const std::string& param1) = 0;
                virtual Json::Value shh_getFilterChanges(const std::string& param1) = 0;
                virtual Json::Value shh_getMessages(const std::string& param1) = 0;
        };

    }
}
#endif //JSONRPC_CPP_STUB_DEV_RPC_WHISPERFACE_H_
