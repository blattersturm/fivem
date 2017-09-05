#include "StdInc.h"
#include <ClientHttpHandler.h>

#include <ClientRegistry.h>
#include <GameServer.h>

#include <ServerInstanceBase.h>

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
		auto gameServer = instance->GetComponent<fx::GameServer>();

		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("getDeferState", [=](const std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request, const std::function<void(const json&)>& cb)
		{
			auto guidIt = postMap.find("guid");
			auto tokenIt = postMap.find("token");

			if (tokenIt == postMap.end() || guidIt == postMap.end())
			{
				cb(json::object({ { "error", "fields missing" } }));
				return;
			}

			// get the client
			auto client = clientRegistry->GetClientByGuid(guidIt->second);

			if (!client)
			{
				cb(json::object({ { "error", "unknown client" } }));
				return;
			}

			// verify the token
			if (client->GetConnectionToken() != tokenIt->second)
			{
				cb(json::object({ { "error", "unknown token" } }));
				return;
			}

			client->Touch();
			client->SetData("canBeDead", true);

			// set a callback to return to the client
			auto returnedCb = std::make_shared<bool>(false);

			auto returnCb = [=]()
			{
				// has already run?
				if (*returnedCb)
				{
					return;
				}

				*returnedCb = true;

				// send the state to the client, if we have any
				auto& state = client->GetData("deferralState");

				if (state.has_value())
				{
					cb(std::any_cast<json>(state));
				}
				else
				{
					cb(json::object());
				}

				// unset the deferral callback
				client->SetData("deferralCallback", std::any{});
			};

			client->SetData("deferralCallback", std::function<void()>{returnCb});

			// set a timer to return for sure after 2 seconds
			gameServer->DeferCall(2000, returnCb);
		});
	}, 5000);
});
