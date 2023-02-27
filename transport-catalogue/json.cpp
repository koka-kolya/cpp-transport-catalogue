#include "json.h"

using namespace std;

namespace json {

namespace {

std::string MakeStringLiteral (istream& input) {
	std::string output;
	while(std::isalpha(input.peek())){
		output.push_back(static_cast<char>(input.get()));
	}
	return output;
}

//////////////////////    LOADERS    //////////////////////
Node LoadNode(istream& input);

Node LoadNull(istream& input) {
	if (MakeStringLiteral(input) != "null"s) {
		throw ParsingError("Value is not null");
	}
	return Node{};
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(std::istream& input) {
	auto it = std::istreambuf_iterator<char>(input);
	auto end = std::istreambuf_iterator<char>();
	std::string s;
	while (true) {
		if (it == end) {
			// Поток закончился до того, как встретили закрывающую кавычку?
			throw ParsingError("String parsing error");
		}
		const char ch = *it;
		if (ch == '"') {
			// Встретили закрывающую кавычку
			++it;
			break;
		} else if (ch == '\\') {
			// Встретили начало escape-последовательности
			++it;
			if (it == end) {
				// Поток завершился сразу после символа обратной косой черты
				throw ParsingError("String parsing error");
			}
			const char escaped_char = *(it);
			// Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
			switch (escaped_char) {
				case 'n':
					s.push_back('\n');
					break;
				case 't':
					s.push_back('\t');
					break;
				case 'r':
					s.push_back('\r');
					break;
				case '"':
					s.push_back('"');
					break;
				case '\\':
					s.push_back('\\');
					break;
				default:
					// Встретили неизвестную escape-последовательность
					throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
			}
		} else if (ch == '\n' || ch == '\r') {
			// Строковый литерал внутри- JSON не может прерываться символами \r или \n
			throw ParsingError("Unexpected end of line"s);
		} else {
			// Просто считываем очередной символ и помещаем его в результирующую строку
			s.push_back(ch);
		}
		++it;
	}
//	std::cout << s << std::endl;
	return Node(std::move(s));
}

Node LoadDict(istream& input) {
	Dict result {};

	//проверка потока на корректность
	char c;
	if (!(input >> c) ) {
		throw ParsingError("Incorrect JSON"s);
	} else if (c == '}') { // признак пустого массива
		return Node(result);
	} else { input.putback(c); }

	for (char c; input >> c && c != '}';) {
		if (c == ',' || c== ':') {
			input >> c;
		}
		string key = LoadString(input).AsString();
		input >> c;
		result.insert({std::move(key), LoadNode(input)});
	}

	return Node(std::move(result));
}

Node LoadArray(istream& input) {
	Array result{};
	char c;

	//проверка потока на корректность
	if (!(input >> c) ) {
		throw ParsingError("Incorrect JSON"s);
	} else if (c == ']') { // признак пустого массива
		return Node(result);
	} else { input.putback(c); }

	for (char c; input >> c && c != ']';) {
		if (c != ',') {
			input.putback(c);
		}
		result.push_back(LoadNode(input));
	}

	return Node(std::move(result));
}

Node LoadBool (istream& input) {
	std::string bool_type= std::move(MakeStringLiteral(input));
	if (bool_type == "true"s) {
		return Node(true);
	} else if (bool_type == "false"s) {
		return Node(false);
	}
	throw ParsingError("Incorrect boolean value"s);
}

Node LoadNumber(std::istream& input) {
	using namespace std::literals;

	std::string parsed_num;

	// Считывает в parsed_num очередной символ из input
	auto read_char = [&parsed_num, &input] {
		parsed_num += static_cast<char>(input.get());
		if (!input) {
			throw ParsingError("Failed to read number from stream"s);
		}
	};

	// Считывает одну или более цифр в parsed_num из input
	auto read_digits = [&input, read_char] {
		if (!std::isdigit(input.peek())) {
			throw ParsingError("A digit is expected"s);
		}
		while (std::isdigit(input.peek())) {
			read_char();
		}
	};

	if (input.peek() == '-') {
		read_char();
	}
	// Парсим целую часть числа
	if (input.peek() == '0') {
		read_char();
		// После 0 в JSON не могут идти другие цифры
	} else {
		read_digits();
	}

	bool is_int = true;
	// Парсим дробную часть числа
	if (input.peek() == '.') {
		read_char();
		read_digits();
		is_int = false;
	}

	// Парсим экспоненциальную часть числа
	if (int ch = input.peek(); ch == 'e' || ch == 'E') {
		read_char();
		if (ch = input.peek(); ch == '+' || ch == '-') {
			read_char();
		}
		read_digits();
		is_int = false;
	}

	try {
		if (is_int) {
			// Сначала пробуем преобразовать строку в int
			try {
				return std::stoi(parsed_num);
			} catch (...) {
				// В случае неудачи, например, при переполнении,
				// код ниже попробует преобразовать строку в double
			}
		}
		return std::stod(parsed_num);
	} catch (...) {
		throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
	}
}

Node LoadNode(istream& input) {

	char c;

	if (!(input >> c)) {
		throw ParsingError("Incorrect JSON. Input is empty."s); // уточнить необходимость проверки
	}
	if (c == ']' || c == '}') {
		throw ParsingError("Incorrect JSON. Container starts with symbol: " + std::string(&c));
	}

	if (c == '[') {
//		input.putback(c);
		return LoadArray(input);
	} else if (c == '{') {
//		input.putback(c);
		return LoadDict(input);
	} else if (c == '"') {
//		input.putback(c);
		return LoadString(input);
	} else if (c == 'n') {
		input.putback(c);
		return LoadNull(input);
	} else if (c == 'f') {
		input.putback(c);
		return LoadBool(input);
	} else if (c == 't') {
		input.putback(c);
		return LoadBool(input);
	}
	else {
		input.putback(c);
		return LoadNumber(input);
	}
}

}  // namespace

//////////////////////    NODE    //////////////////////

Node::Node(Node::Value val)
	: value_(std::holds_alternative<std::nullptr_t>(val) ? nullptr : val) {
}
Node::Node(std::nullptr_t)
	: value_(nullptr){
}
Node::Node(Array arr)
	: value_(std::move(arr)){
}
Node::Node(Dict dict)
	: value_(std::move(dict)){
}
Node::Node(bool val)
	:value_(val){
}
Node::Node(int val)
	: value_(std::move(val)){
}
Node::Node(double val)
	: value_(std::move(val)){
}
Node::Node(std::string val)
	: value_(std::move(val)){
}

bool Node::IsInt() const {
	return std::holds_alternative<int>(value_);
}
bool Node::IsDouble() const {
	return std::holds_alternative<double>(value_) ||
		   std::holds_alternative<int>(value_);
}
bool Node::IsPureDouble() const {
	return std::holds_alternative<double>(value_);
}
bool Node::IsBool() const {
	return std::holds_alternative<bool>(value_);
}
bool Node::IsString() const {
	return std::holds_alternative<std::string>(value_);
}
bool Node::IsNull() const {
	return std::holds_alternative<std::nullptr_t>(value_);
}
bool Node::IsArray() const {
	return std::holds_alternative<Array>(value_);
}
bool Node::IsMap() const {
	return std::holds_alternative<Dict>(value_);
}

const Node::Value& Node::GetValue() const {
	return value_;
}

bool Node::operator==(const Node& rhs) const {
	return GetValue() == rhs.GetValue();
}
bool Node::operator!=(const Node& rhs) const {
	return GetValue() != rhs.GetValue();
}

const Array& Node::AsArray() const {
	if (!this->IsArray()) { throw std::logic_error("Not array value"); }
	return std::get<Array>(value_);
}

bool Node::AsBool() const {
	if (!this->IsBool()) { throw std::logic_error("Not bool value"); }
	return std::get<bool>(value_);
}

double Node::AsDouble() const {
	if (!this->IsDouble()) { throw std::logic_error("Not double value"); }
	if (this->IsInt()) { return static_cast<double>(std::get<int>(value_)); }
	return std::get<double>(value_);
}

const Dict& Node::AsMap() const {
	if (!this->IsMap()) { throw std::logic_error("Not Map"); }
	return std::get<Dict>(value_);
}

int Node::AsInt() const {
	if (!Node::IsInt()) { throw std::logic_error("Not int value"); }
	return std::get<int>(value_);
}

const string& Node::AsString() const {
	if (!this->IsString()) { throw std::logic_error("Not string value");}
	return std::get<std::string>(value_);
}

//////////////////////    DOCUMENT    //////////////////////

Document::Document(Node root)
	: root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
	return root_;
}

Document Load(istream& input) {
	return Document{LoadNode(input)};
}

bool Document::operator==(const Document& doc) const {
	return root_ == doc.GetRoot();
}

bool Document::operator!=(const Document& doc) const  {
	return root_ != doc.GetRoot();
}

//////////////////////    PRINTERS    //////////////////////

void PrintContext::PrintIndent() const {
	for (int i = 0; i < indent; ++i) {
		out.put(' ');
	}
}

// Возвращает новый контекст вывода с увеличенным смещением
PrintContext PrintContext::Indented() const {
	return {out, indent_step, indent_step + indent};
}

// Шаблон, подходящий для вывода double и int
template <typename T>
void PrintValue(const T& value, const PrintContext& ctx) {
	std::ostream& out = ctx.out;
	out << value;
}

void PrintValue(const Node& /*node*/, const PrintContext& /*ctx*/) {}

void PrintValue(const std::nullptr_t, const PrintContext& ctx) {
	std::ostream& out = ctx.out;
	out << "null"sv;
}
void PrintValue(const int value, const PrintContext& ctx) {
	std::ostream& out = ctx.out;
	out << std::to_string(value);
}
void PrintValue(const double value, const PrintContext& ctx) {
	std::ostream& out = ctx.out;
	out << value;
}
void PrintValue(const bool value, const PrintContext& ctx) {
	std::ostream& out = ctx.out;
	out << (value ? "true"s : "false"s);
}

void PrintValue(const std::string& str, const PrintContext& ctx) {
	std::ostream& out = ctx.out;
	out << "\""sv;
	for (const char c : str) {
		if (c == '\\') {
			out << "\\\\"sv;
		} else if (c == '"') {
			out << "\\\""sv;
		} else if (c == '\n') {
			out << "\\n"sv;
		} else if (c == '\r') {
			out << "\\r"sv;
		} else if (c == '\t') {
			out << "\t"sv;
		} else {
			out << c;
		}
	}
	out << "\""sv;
}

void PrintValue(const Array& arr, const PrintContext& ctx) {
	std::ostream& out = ctx.out;
	out << "[\n"sv;
	bool is_first = true;
	for (const auto& node : arr) {
		if (is_first) {
			is_first = false;
		} else {
			out << ","sv;
		}
		ctx.Indented().PrintIndent();
		PrintNode(node, ctx.Indented());
	}
	ctx.Indented().PrintIndent();
	out << "\n]"sv;
}

void PrintValue(const Dict& dict, const PrintContext& ctx) {
	std::ostream& out = ctx.out;
	out << "{"sv;
	out << "\n"sv;
	bool is_first = true;
	for (const auto& [key, value] : dict) {
		if (is_first) {
			is_first = false;
		} else {
			out << ",\n"sv;
		}
		ctx.Indented().PrintIndent();
		PrintNode(key, ctx);
		out << ": "sv;
		PrintNode(value, ctx.Indented());
	}
	out << "\n"sv;
	out << "}"sv;
	out << "\n"sv;
}

void PrintNode(const Node& node, const PrintContext& ctx) {
	std::visit(
		[&ctx](const auto& value){ PrintValue(value, ctx); },
		node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
	PrintNode(doc.GetRoot(), PrintContext{output});
}

}  // namespace json
