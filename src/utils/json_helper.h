#pragma once
#include <jsoncpp/json/json.h>
#include <string>

class JsonHelper {
public:
    static bool parseJson(const std::string& input, Json::Value& root) {
        Json::Reader reader;
        return reader.parse(input, root);
    }

    static std::string toString(const Json::Value& root) {
        Json::FastWriter writer;
        return writer.write(root);
    }
}; 