#pragma once

#include <map>
#include <string>

// Lightweight Lua-ish parser for simple config files (supports variables + basic arithmetic)
class SimpleLuaParser {
public:
    std::map<std::string, std::map<std::string, std::string>> tables;
    std::map<std::string, std::string> variables;

    void Parse(const std::string& content);
    std::string GetTableValue(const std::string& table, const std::string& key, const std::string& def = "") const;
    static std::string NumberToString(double v);

private:
    static std::string Trim(const std::string& s);
    static std::string RemoveTrailingComma(const std::string& s);
    std::string EvaluateExpression(const std::string& expr);
    std::string EvaluateArithmetic(const std::string& expr);
    double ToNumber(const std::string& s);
};

int ParseLuaInt(const std::string& value, int defaultVal);
float ParseLuaFloat(const std::string& value, float defaultVal);
bool ParseLuaBool(const std::string& value, bool defaultVal);
float ParseFloat(const std::string& value, float defaultVal);
