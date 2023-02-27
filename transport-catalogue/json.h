#pragma once

#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>


namespace json {
using namespace std::literals;
class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
	using runtime_error::runtime_error;
};

class Node {
public:
	using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

	Node() = default;
	Node(std::nullptr_t);
	Node(Value val);
	Node(Array arr);
	Node(Dict dict);
	Node(bool val);
	Node(int val);
	Node(double val);
	Node(std::string val);

	bool IsInt() const;
	bool IsDouble() const;
	bool IsPureDouble() const;
	bool IsBool() const;
	bool IsString() const;
	bool IsNull() const;
	bool IsArray() const;
	bool IsMap() const;

	const Value& GetValue() const;
	bool operator==(const Node& rhs) const;
	bool operator!=(const Node& rhs) const;

	int AsInt () const;
	bool AsBool() const;
	double AsDouble() const;
	const std::string& AsString() const;
	const Array& AsArray() const;
	const Dict& AsMap() const;
private:
	Value value_ = nullptr;
};

class Document {
public:
	explicit Document(Node root);
	const Node& GetRoot() const;
	bool operator==(const Document& doc) const;
	bool operator!=(const Document& doc) const;
private:
	Node root_;
};

Document Load(std::istream& input);

struct PrintContext {
	std::ostream& out;
	int indent_step = 4;
	int indent = 0;
	void PrintIndent() const;
	// Возвращает новый контекст вывода с увеличенным смещением
	PrintContext Indented() const;
};

template <typename T>
void PrintValue(const T& value, const json::PrintContext& ctx);
void PrintValue(const json::Node& node, const json::PrintContext& ctx);
void PrintValue(const std::nullptr_t, const json::PrintContext& ctx);
void PrintValue(const int value, const json::PrintContext& ctx);
void PrintValue(const double value, const json::PrintContext& ctx);
void PrintValue(const bool value, const json::PrintContext& ctx);
void PrintValue(const std::string& str, const json::PrintContext& ctx);
void PrintValue(const json::Array& arr, const json::PrintContext& ctx);
void PrintValue(const json::Dict& dict, const json::PrintContext& ctx);
void PrintNode(const json::Node& node, const json::PrintContext& ctx);
void Print(const json::Document& doc, std::ostream& output);

}  // namespace json
