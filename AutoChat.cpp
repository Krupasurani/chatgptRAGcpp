#include "stdafx.h"

#include "utils.h"
#include <curl\curl.h>
#include "AutoChat.h"
#include "utils.h"
#include "REST\JSON.h"
#include "REST\JSONValue.h"
#include "nlohmann/json.hpp"
#include <codecvt>
#include <locale>
#include <string>

// ...
std::wstring gSefariaPrompt{L"You are an expert in the Sefaria API. Please convert this question into a Sefaria-friendly reference (in the form of 'Book_Verse' like 'Genesis_1', 'Exodus_3:14', etc.)"};
std::wstring gExplanationRequest{L""};
std::string gAPIKey{""};
std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8conv;

using json = nlohmann::json;

/*

    Developed by Michael Haephrati
    https://www.securedglobe.net

    April 2023

*/
void SaveConfigData(const std::wstring& configFilePath, const ConfigData& configData)
{
	gSefariaPrompt = configData.sefariaPrompt;
	gExplanationRequest =  configData.explanationRequest;
	gAPIKey = configData.apiKey;

	std::ofstream configFile(convertWstringToString(configFilePath)); // Convert wstring to string for std::ofstream
	if (!configFile.is_open())
	{
		WriteLogFile(L"Failed to open config file for writing: %s", configFilePath.c_str());
		return;
	}

	json jsonData;

	try
	{
		// Write mandatory fields
		jsonData["openai_api_key"] = configData.apiKey;
		jsonData["sefaria_prompt"] = convertWstringToString(configData.sefariaPrompt);
		jsonData["explanation_request"] = convertWstringToString(configData.explanationRequest);

		// Save the JSON to file
		configFile << jsonData.dump(4); // The '4' here specifies indentation for pretty-printing

		WriteLogFile(L"Config data successfully saved to: %s", configFilePath.c_str());
	}
	catch (const std::exception& e)
	{
		WriteLogFile(L"Error saving config file: %s", convertStringToWstring(e.what()).c_str());
	}
}

ConfigData LoadConfigData(const std::wstring& configFilePath)
{
	std::ifstream configFile(convertWstringToString(configFilePath)); // Convert wstring to string for std::ifstream
	if (!configFile.is_open())
	{
		WriteLogFile(L"Failed to open config file: %s", configFilePath.c_str());
		return {};
	}

	ConfigData configData;
	json jsonData;

	try
	{
		configFile >> jsonData;

		// Read mandatory fields
		if (jsonData.contains("openai_api_key"))
		{
			configData.apiKey = jsonData["openai_api_key"].get<std::string>();
		}
		else
		{
			WriteLogFile(L"Missing 'openai_api_key' in config file.");
		}

		if (jsonData.contains("sefaria_prompt"))
		{
			configData.sefariaPrompt = convertStringToWstring(jsonData["sefaria_prompt"].get<std::string>());
		}
		else
		{
			WriteLogFile(L"Missing 'sefaria_prompt' in config file.");
		}
		if (jsonData.contains("explanation_request"))
		{
			configData.explanationRequest = convertStringToWstring(jsonData["explanation_request"].get<std::string>());
		}
		else
		{
			WriteLogFile(L"Missing 'explanation_request' in config file.");
		}
		gSefariaPrompt = configData.sefariaPrompt;
		gExplanationRequest = configData.explanationRequest;
		gAPIKey = configData.apiKey;

	}
	catch (const std::exception& e)
	{
		WriteLogFile(L"Error reading config file: %s", convertStringToWstring(e.what()).c_str());
	}

	return configData;
}

size_t callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	size_t realsize = size * nmemb;
	std::wstring* response = static_cast<std::wstring*>(userdata);
	std::string utf8str(ptr, realsize);

	std::wstring intermediate_response = convertStringToWstring(utf8str);
	if (intermediate_response.back() == '\0')
		intermediate_response.pop_back();

	response->append(intermediate_response); 

	return realsize;
}


CURLcode SG_API::perform_request(string& payload, wstring& response)
{
	// Setting cURL options
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.length());
	curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, payload.c_str());
	curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	return (curl_easy_perform(curl));
}

// Helper function to get the directory of the executable
std::wstring GetExecutableDir()
{
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(NULL, buffer, MAX_PATH); // Use wide-character version
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, pos);
}

// Updated SG_API constructor
void SG_API::UpdateSettingsFromConfig()
{
	// Construct the full path to config.json
	std::wstring exeDir = GetExecutableDir();
	std::wstring configPath = exeDir + L"\\config.json";

	// Load API key from the constructed config path
	std::string apiKey;
	ConfigData config = LoadConfigData(configPath);
	if (config.apiKey.empty())
	{
		WriteLogFile(L"Error: Failed to load OpenAI API key from %s", configPath.c_str());
		return;
	}
	gSefariaPrompt=config.sefariaPrompt;
	gExplanationRequest = config.explanationRequest;
	gAPIKey = config.apiKey;
}
SG_API::SG_API()
{
	curl = curl_easy_init();
	if (!curl)
	{
		WriteLogFile(L"Failed to initialize libcurl");
		return;
	}

	UpdateSettingsFromConfig();
	headers = curl_slist_append(headers, "Content-Type: application/json");
	std::string authHeader = "Authorization: Bearer " + gAPIKey;
	headers = curl_slist_append(headers, authHeader.c_str());
	entire_converstaion_string = prompt_start;
	wstring quotaResponse;
	if (!checkQuota(quotaResponse))
	{
		MessageBox(NULL,L"Billing or quota issue detected, unable to proceed.",L"SGGPT",MB_OK);
		return;  // Stop if there are issues
	}
}


SG_API::~SG_API()
{
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);
}

// Function to check if there are billing or quota issues
bool SG_API::checkQuota(std::wstring& response)
{
	string apiCheckUrl = "https://api.openai.com/v1/engines"; // Lightweight API endpoint to check for quota
	curl_easy_setopt(curl, CURLOPT_URL, apiCheckUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	CURLcode res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		WriteLogFile(L"Error checking quota: %s", curl_easy_strerror(res));
		return false;
	}

	// Check if the response indicates any quota issues
	try
	{
		json j = json::parse(response);
		if (j.contains("error") && j["error"]["code"] == "insufficient_quota")
		{
			WriteLogFile(L"Insufficient quota: %S", j["error"]["message"].get<std::string>().c_str());
			return false;
		}
	}
	catch (const std::exception& e)
	{
		WriteLogFile(L"Error parsing quota response: %S", e.what());
		return false;
	}

	return true; // If no quota issues were found
}
bool SG_API::SG_GPT_GENERIC(std::wstring request, std::wstring& response, wstring w_model, float temperature, int max_tokens, int n, wstring w_stop)
{
	WriteLogFile(L"SG_GPT_GENERIC()\nrequest = '%s'",request.c_str());
	if (!curl)
	{
		curl = curl_easy_init();
		if (!curl)
		{
			WriteLogFile(L"Failed to initialize libcurl");
			return false;
		}
	}

	response.clear();

	string string_request = convertWstringToString(request);

	if (string_request.back() == '\0')
		string_request.pop_back();

	string string_response;

	string model = convertWstringToString(w_model);

	if (model.back() == '\0')
		model.pop_back();

	string stop = convertWstringToString(w_stop);

	if (stop.back() == '\0')
		stop.pop_back();

	// Invalid max tokens value passed in, setting to default
	if (max_tokens > 4800 || max_tokens < 1)
		max_tokens = 4800;

	// Invalid n value passed in, setting to default
	if (n < 1)
		n = 1;

		// Invalid temperature value passed in, setting to default
	if (temperature > 1.0 || temperature < 0.0)
		temperature = 0.7;

	entire_converstaion_string += "request: " + string_request + "\n";

	json data;

	try
	{
		data =
		{
			{"messages", json::array({{ {"role", "user"}, {"content", entire_converstaion_string} }})},
			{"model", model},
			{"temperature", temperature},
			{"max_tokens", max_tokens},
			{"n", n},
			{"stop", stop}
		};
	}
	catch (const std::exception& e)
	{
		WriteLogFile(L"Error creating JSON payload: %S", e.what());
		return false;
	}

	string payload;
	try
	{
		payload = data.dump();
	}

	catch (const std::exception& e)
	{
		WriteLogFile(L"Error dumping JSON payload to string: %S", e.what());
		return false;
	}

	CURLcode res = perform_request(payload, response);

	if (res != CURLE_OK)
	{
		WriteLogFile(L"curl_easy_perform() failed: %s", curl_easy_strerror(res));
		return false;
	}

	try
	{
		// Parse the JSON response
		json j = json::parse(response);
		string intermediate_response = j["choices"][0]["message"]["content"];
		string_response = intermediate_response;
		string_response = omitSubstring(string_response, "response:");
		response = convertStringToWstring(string_response);
		WriteLogFile(L"Response from Chat GPT: '%s'", response.c_str());
	}

	catch (const std::exception& e)
	{
		WriteLogFile(L"Error parsing JSON response: %S", e.what());
		return false;
	}

	if (string_response.back() == '\0')
		string_response.pop_back();
	entire_converstaion_string += string_response + "\n";

	return true;
}

// New FetchSefariaText method
std::wstring SG_API::FetchSefariaText(const std::wstring& query)
{
	// Trim leading and trailing whitespace from the query
	std::wstring trimmedQuery = query;
	trimmedQuery.erase(0, trimmedQuery.find_first_not_of(L" \t"));
	trimmedQuery.erase(trimmedQuery.find_last_not_of(L" \t") + 1);

	WriteLogFile(L"FetchSefariaText() '%s'", query.c_str());

	// Construct the URL with the trimmed query
	std::wstring url = L"https://www.sefaria.org/api/texts/" + trimmedQuery;

	WriteLogFile(L"URL for query: '%s'",url.c_str());
	std::wstring response;

	CURLcode res = perform_request(url, response); // Use perform_request for fetching data

	if (res != CURLE_OK)
	{
		WriteLogFile(L"Failed to fetch Sefaria text: %S", curl_easy_strerror(res));
		return L"";
	}
	WriteLogFile(L"Response: '%s'",response.c_str());
	return response;
}

#include <regex> // For regex_replace to handle HTML tags

// Helper function to remove HTML tags
std::wstring RemoveHtmlTags(const std::wstring& input)
{
	// Regular expression to match HTML tags
	std::wregex htmlTagPattern(L"<[^>]*>");
	return std::regex_replace(input, htmlTagPattern, L"");
}

std::wstring SG_API::GenerateFriendlyTextFromResponse(const std::wstring& response)
{
	std::wstring friendlyText;

	try
	{
		// Parse the JSON response
		json j = json::parse(convertWstringToString(response));

		// Check if the response contains the "text" field
		if (j.contains("text") && j["text"].is_array() && !j["text"].empty())
		{
			// Join all text parts into a single string
			std::wstring combinedText;
			for (const auto& part : j["text"])
			{
				// Remove HTML tags from each part before adding it to the combined text
				combinedText += RemoveHtmlTags(convertStringToWstring(part.get<std::string>()));
			}
			friendlyText = combinedText;
		}
		else
		{
			// Handle the case where the "text" field is missing or empty
			WriteLogFile(L"Error: 'text' field missing or empty in response.");
			friendlyText = L"Error: Unable to parse response.";
		}
	}
	catch (const std::exception& e)
	{
		WriteLogFile(L"Error parsing response to user-friendly text: %S", e.what());
		friendlyText = L"Error processing response.";
	}

	return friendlyText;
}


// Updated perform_request to handle both APIs
CURLcode SG_API::perform_request(const std::wstring& url, std::wstring& response)
{
	curl_easy_setopt(curl, CURLOPT_URL, convertWstringToString(url).c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	return curl_easy_perform(curl);
}