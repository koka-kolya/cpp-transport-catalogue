#include "json_builder.h"

namespace json {
using namespace std::literals;

/////////////////// BUILDER ///////////////////
Builder::Builder() {
	nodes_stack_.emplace_back(&root_);
}

KeyItemContext Builder::Key(std::string key) {
	if (nodes_stack_.empty()) {
		throw std::logic_error("Fail-add the key to the finished object"s);
	} else if (nodes_stack_.back()->IsDict()) {
		//кладем на стек указатель на узел дерева мапы m[key]
		nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsDict())[key]);
	} else {
		throw std::logic_error("Fail-add key not to dictionary"s);
	}
	return AsBuilder();
}

Builder& Builder::Value(Node node) { // используем сигнатуру Value(Node), поскольку класс Node
									 // наследуется от std::variant, включающий типы всех значений
									 // Node::Value
	if (nodes_stack_.empty() || (!nodes_stack_.back()->IsArray() && !nodes_stack_.back()->IsNull())) {
		throw std::logic_error("Fail-attempt to add value"s);
	} else if (nodes_stack_.back()->IsArray()) {
		// если на стеке указатель на массив, добавляем в него ноду
		const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(node);
	} else {
		*nodes_stack_.back() = node; // записываем по указателю на стеке полученную ноду
		nodes_stack_.pop_back();	 // убираем указатель на записанную ноду со стека
	}
	return AsBuilder();
}

DictItemContext Builder::StartDict() {
	if (nodes_stack_.empty() || (!nodes_stack_.back()->IsArray() && !nodes_stack_.back()->IsNull())) {
		throw std::logic_error("Fail-start dict"s);
	} else {
		StartContainer(Dict());
	}
	return AsBuilder();
}

ArrayItemContext Builder::StartArray() {
	if (nodes_stack_.empty()) {
		throw std::logic_error("Fail-start array"s);
	} else {
		StartContainer(Array());
	}
	return AsBuilder();
}

Builder &Builder::EndDict() {
	EndContainer();
	return AsBuilder();
}

Builder& Builder::EndArray() {
	EndContainer();
	return AsBuilder();
}

json::Node Builder::Build() {
	if (!nodes_stack_.empty()) {
		throw std::logic_error("Fail to build before end of object"s);
	}
	return root_;
}

Builder::~Builder(){
	ClearBuilder();
}

void Builder::StartContainer(Node node) {
	if (nodes_stack_.back()->IsArray()) {
		const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(node);
		nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).back());
	} else {
		*nodes_stack_.back() = node;
	}
}

void Builder::EndContainer() {
	std::string container_type = nodes_stack_.back()->IsArray() ? "array"s : "dictionary"s;
	if (nodes_stack_.empty()) {
		throw std::logic_error("Fail-start "s + container_type);
	} else if (nodes_stack_.back()->IsDict() || nodes_stack_.back()->IsArray()) {
		// Можем сделать такое условие, потому что правильная последовательность методов гарантирована
		// на этапе компиляции. Мы не вызовем случайно метод закрытия для типа контейнера,
		// отличного от того, что был открыт. Такой код не скомплилируется.
		nodes_stack_.pop_back(); // "закрываем" контейнер, убираем указатель со стека
	} else {
		throw std::logic_error("Fail-end "s + container_type);
	}
}

void Builder::ClearBuilder() {
	nodes_stack_.clear();
	root_ = nullptr;
}

Builder& Builder::AsBuilder() {
	return static_cast<Builder&>(*this);
}

/////////////////// ITEM CONTEXTs ///////////////////
//Item - Parent class
ItemContext::ItemContext(Builder& builder)
	: builder_(builder) {
}
KeyItemContext ItemContext::Key(std::string key) {
	return builder_.Key(std::move(key));
}
Builder &ItemContext::Value(json::Node node) {
	return builder_.Value(std::move(node));
}
DictItemContext ItemContext::StartDict() {
	return builder_.StartDict();
}
ArrayItemContext ItemContext::StartArray() {
	return builder_.StartArray();
}
Builder &ItemContext::EndDict() {
	return builder_.EndDict();
}
Builder &ItemContext::EndArray() {
	return builder_.EndArray();
}

ItemContext::~ItemContext() {
	builder_.ClearBuilder();
}

//Key
KeyItemContext::KeyItemContext(Builder& builder) : ItemContext(builder) {}

//Dict
DictItemContext::DictItemContext(Builder& builder) : ItemContext(builder) {}

//Array
ArrayItemContext::ArrayItemContext(Builder& builder) : ItemContext(builder) {}
ArrayItemContext ArrayItemContext::Value(json::Node node) {
	return ItemContext::Value(std::move(node));
}

//Value
ValueItemContext::ValueItemContext(class Builder& builder) : ItemContext(builder) {}
ValueItemContext KeyItemContext::Value(json::Node node) {
	return ItemContext::Value(std::move(node));
}

} // namespace json
