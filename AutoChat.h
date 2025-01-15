#pragma once
#include <string>
#include <vector>
#include <utility>
#include "include/curl/curl.h"

extern std::wstring gSefariaPrompt;
extern std::wstring gExplanationRequest;
std::wstring GetExecutableDir();

/*
Sample json file

{
	"openai_api_key": "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",
	"sefaria_prompt": "You are an expert in the Sefaria API. Please convert this question into a Sefaria-friendly reference (in the form of 'Book_Verse' like 'Genesis_1', 'Exodus_3:14', etc.):",
	"explanation_request": "Here is a response received from sefaria API. Please provide a brief explanation (max 15 words) summarizing it clearly and concisely:\n"
}

*/
struct ConfigData
{
	std::string apiKey;
	std::wstring sefariaPrompt;
	std::wstring explanationRequest;
};
ConfigData LoadConfigData(const std::wstring& configFilePath);
void SaveConfigData(const std::wstring& configFilePath, const ConfigData& configData);

std::string ObtainURL(std::string Buffer);
std::string ObtainStatus(std::string Buffer);
class SG_API
{
private:
	using SingleExchange = std::pair<std::string, std::string>;
	using Conversation = std::vector<SingleExchange>;

	int conversation_exchange_limit{ 100 };
	std::string prompt_start{ "You are a chatbot. I want to have a conversation with you where you can remember the context between multiple requests. To do that, I will send all previous request prompts and your corresponding responds, please use those as context. All the previous request prompts and the current will have the 'request: ' before it, and all your corresponding responses will have 'response: ' before it. Your job is to respond to only the latest request. Do not add anything by yourself, just response to the latest request. Starting now\n\n" };
	std::string entire_converstaion_string;

	Conversation entire_conversation;
	CURL* curl;
	struct curl_slist* headers;

	CURLcode perform_request(std::string& payload, std::wstring& response);

public:
	SG_API();
	~SG_API();
	void UpdateSettingsFromConfig();
	bool checkQuota(std::wstring& response);
	std::wstring GenerateFriendlyTextFromResponse(const std::wstring& response);
	bool SG_GPT_GENERIC(std::wstring request, std::wstring& response, std::wstring w_model = L"gpt-3.5-turbo", float temperature = 0.7, int max_tokens = 2400, int n = 1, std::wstring w_stop = L"");
	CURLcode perform_request(const std::wstring& url, std::wstring& response);
	std::wstring FetchSefariaText(const std::wstring& query);


};