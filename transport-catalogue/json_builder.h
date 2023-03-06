#pragma once

#include "json.h"
#include <string>
#include <utility>
namespace json {

class ItemContext;
class KeyItemContext;
class DictItemContext;
class ArrayItemContext;
class ValueItemContext;

class Builder {
public:
	Builder();
	KeyItemContext Key(std::string key);
	Builder& Value(json::Node node);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
	Builder& EndDict();
	Builder& EndArray();
	json::Node Build();
	void ClearBuilder();
	~Builder();
private:
	Node root_;
	std::vector<Node*> nodes_stack_;

	Builder& AsBuilder();
	void StartContainer(Node node);
	void EndContainer();
};

class ItemContext {
public:
	ItemContext(Builder &builder);
	KeyItemContext Key(std::string key);
	Builder& Value(json::Node node);
	DictItemContext StartDict();
	ArrayItemContext StartArray();
	Builder& EndDict();
	Builder& EndArray();
	~ItemContext();
private:
	Builder& builder_;
};

class KeyItemContext : public ItemContext {
public:
	KeyItemContext(Builder& builder);
	KeyItemContext Key(std::string key) = delete;
	ValueItemContext Value(json::Node node);
	Builder& EndArray() = delete;
	Builder& EndDict() = delete;
};

class DictItemContext : public ItemContext {
public:
	DictItemContext(Builder& builder);
	Builder& Value(json::Node node) = delete;
	DictItemContext StartDict() = delete;
	ArrayItemContext StartArray() = delete;
	Builder& EndArray() = delete;

};

class ArrayItemContext : public ItemContext {
public:
	ArrayItemContext(Builder& builder);
	ArrayItemContext Value(json::Node node);
	Builder& EndDict() = delete;
	KeyItemContext Key(std::string key) = delete;
};

class ValueItemContext : public ItemContext {
public:
	ValueItemContext(Builder& builder);
	Builder& Value(json::Node node) = delete;
	DictItemContext StartDict() = delete;
	ArrayItemContext StartArray() = delete;
	Builder& EndArray() = delete;
};

} // namespace json
