#include "luaUtils.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>
#include <cctype>

void SimpleLuaParser::Parse(const std::string& content) {
    std::istringstream iss(content);
    std::string line;
    std::string currentTable;

    while (std::getline(iss, line)) {
        size_t commentPos = line.find("--");
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);
        line = Trim(line);
        if (line.empty()) continue;

        if (line.rfind("local ", 0) == 0 && line.find(" = ") != std::string::npos) {
            size_t eqPos = line.find(" = ");
            std::string name = Trim(line.substr(6, eqPos - 6));
            std::string value = Trim(RemoveTrailingComma(line.substr(eqPos + 3)));
            variables[name] = EvaluateExpression(value);
            continue;
        }

        if (line.find(" = {") != std::string::npos) {
            size_t eqPos = line.find(" = {");
            currentTable = Trim(line.substr(0, eqPos));
            tables[currentTable] = {};
            continue;
        }

        if (line == "}") {
            currentTable.clear();
            continue;
        }

        if (!currentTable.empty() && line.find('=') != std::string::npos) {
            size_t eqPos = line.find('=');
            std::string key = Trim(line.substr(0, eqPos));
            std::string value = Trim(RemoveTrailingComma(line.substr(eqPos + 1)));
            tables[currentTable][key] = EvaluateExpression(value);
        }
    }
}

std::string SimpleLuaParser::GetTableValue(const std::string& table, const std::string& key, const std::string& def) const {
    auto it = tables.find(table);
    if (it == tables.end()) return def;
    auto fit = it->second.find(key);
    if (fit == it->second.end()) return def;
    return fit->second;
}

std::string SimpleLuaParser::NumberToString(double v) {
    if (std::floor(v) == v) return std::to_string((long long)v);
    return std::to_string(v);
}

std::string SimpleLuaParser::Trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

std::string SimpleLuaParser::RemoveTrailingComma(const std::string& s) {
    std::string r = Trim(s);
    if (!r.empty() && r.back() == ',') r.pop_back();
    return Trim(r);
}

std::string SimpleLuaParser::EvaluateExpression(const std::string& expr) {
    std::string t = Trim(expr);
    auto varIt = variables.find(t);
    if (varIt != variables.end()) return varIt->second;
    if (t.find_first_of("+-*/") != std::string::npos) return EvaluateArithmetic(t);
    return t;
}

std::string SimpleLuaParser::EvaluateArithmetic(const std::string& expr) {
    const char ops[] = {'*', '/', '+', '-'};
    for (char op : ops) {
        size_t pos = expr.find(op);
        if (pos != std::string::npos && pos > 0 && pos + 1 < expr.size()) {
            std::string left = Trim(expr.substr(0, pos));
            std::string right = Trim(expr.substr(pos + 1));
            double l = ToNumber(left);
            double r = ToNumber(right);
            if (std::isnan(l) || std::isnan(r)) continue;
            double res = 0.0;
            switch (op) {
                case '*': res = l * r; break;
                case '/': if (r == 0) return expr; res = l / r; break;
                case '+': res = l + r; break;
                case '-': res = l - r; break;
            }
            return NumberToString(res);
        }
    }
    return expr;
}

double SimpleLuaParser::ToNumber(const std::string& s) {
    auto varIt = variables.find(s);
    if (varIt != variables.end()) return ToNumber(varIt->second);
    try {
        return std::stod(s);
    } catch (...) {
        return std::numeric_limits<double>::quiet_NaN();
    }
}

int ParseLuaInt(const std::string& value, int defaultVal) {
    try {
        return std::stoi(value);
    } catch (...) {
        return defaultVal;
    }
}

float ParseLuaFloat(const std::string& value, float defaultVal) {
    try {
        return std::stof(value);
    } catch (...) {
        return defaultVal;
    }
}

bool ParseLuaBool(const std::string& value, bool defaultVal) {
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    if (lower == "true") return true;
    if (lower == "false") return false;
    return defaultVal;
}

float ParseFloat(const std::string& value, float defaultVal) {
    try {
        return std::stof(value);
    } catch (...) {
        return defaultVal;
    }
}
