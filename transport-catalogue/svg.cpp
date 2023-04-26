#include "svg.h"
#include <algorithm>
#include <string_view>
#include <string>

namespace svg {

using namespace std::literals;
std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
	switch (line_cap) {
	case StrokeLineCap::BUTT:
	out << "butt"sv;
		break;
	case StrokeLineCap::ROUND:
		out << "round"sv;
		break;
	case StrokeLineCap::SQUARE:
		out << "square"sv;
		break;
	}
	return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
	switch (line_join) {
	case StrokeLineJoin::ARCS:
	out << "arcs"sv;
		break;
	case StrokeLineJoin::BEVEL:
		out << "bevel"sv;
		break;
	case StrokeLineJoin::MITER:
		out << "miter"sv;
		break;
	case StrokeLineJoin::MITER_CLIP:
		out << "miter-clip"sv;
		break;
	case StrokeLineJoin::ROUND:
		out << "round"sv;
		break;
	}
	return out;
}
////////////////////////////////////   RENDER CONTEXT   ///////////////////////////////////////
void Object::Render(const RenderContext& context) const {
	context.RenderIndent();

	// Делегируем вывод тега своим подклассам
	RenderObject(context);

	context.out << std::endl;
}
////////////////////////////////////////   CIRCLE   ///////////////////////////////////////////
Circle& Circle::SetCenter(Point center)  {
	center_ = center;
	return *this;
}

Circle& Circle::SetRadius(double radius)  {
	radius_ = radius;
	return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
	auto& out = context.out;
	out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
	out << "r=\""sv << radius_ << "\" "sv;
	RenderAttrs(context.out);
	out << "/>"sv;
}
////////////////////////////////////      POLYLINE      ///////////////////////////////////////
Polyline& Polyline::AddPoint(Point point) {
	points_.push_back(std::move(point));
	return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
	auto& out = context.out;
	out << "<polyline points=\""sv;
	bool is_first = true;
	for (auto it = points_.begin(); it != points_.end(); ++it) {
		if (!is_first) {
			out << " "sv;
		}
		out << it->x << "," << it->y;
		is_first = false;
	}
	out << "\" "sv;
	RenderAttrs(context.out);
	out << "/>"sv;
}

//////////////////////////////////////      TEXT      /////////////////////////////////////////
// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
	pos_ = std::move(pos);
	return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
	offset_ = std::move(offset);
	return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
	size_ = std::move(size);
	return *this;
}
// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
	font_family_ = std::move(font_family);
	return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
	font_weight_= std::move(font_weight);
	return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
	if (std::any_of(data.begin(), data.end(), [] (const char c) {return c == '&';})) {
		std::string what = "&";
		std::string with = "&amp;";
		Text::ReplaceAllMarks(data, what, with);
	}
	if (std::any_of(data.begin(), data.end(), [] (const char c) {return c == '"';} )) {
		std::string what = "\"";
		std::string with = "&quot;";
		Text::ReplaceAllMarks(data, what, with);
	}
	if (std::any_of(data.begin(), data.end(), [] (const char c) {return c == '\'';})) {
		std::string what = "'";
		std::string with = "&apos;";
		Text::ReplaceAllMarks(data, what, with);
	}
	if (std::any_of(data.begin(), data.end(), [] (const char c) {return c == '<';})) {
		std::string what = "<";
		std::string with = "&lt;";
		Text::ReplaceAllMarks(data, what, with);
	}
	if (std::any_of(data.begin(), data.end(), [] (const char c) {return c == '>';})) {
		std::string what = ">";
		std::string with = "&gt;";
		Text::ReplaceAllMarks(data, what, with);
	}
	data_ = std::move(data);
	return *this;
}

void Text::RenderObject(const RenderContext& context) const {
	auto& out = context.out;
	out					<< "<text ";
	RenderAttrs(context.out);
	out <<				   " x=\""sv;
	out << pos_.x		<< "\" y=\""sv;
	out << pos_.y		<< "\" dx=\""sv;
	out << offset_.x	<< "\" dy=\""sv;
	out << offset_.y	<< "\" font-size=\""sv;
	out << size_;
	if (!font_family_.empty()) {
	out					<< "\" font-family=\""sv;
	out << font_family_;
	}
	if (!font_weight_.empty()) {
	out					<< "\" font-weight=\""sv;
	out << font_weight_ << "\">"sv;
	} else {
	out					<< "\">"sv;
	}
	out << data_		<< "</text>"sv;
}

void Text::ReplaceAllMarks(std::string& inout, std::string_view what, std::string_view with) const {
	for (std::string::size_type pos{};
		 inout.npos != (pos = inout.find(what.data(), pos, what.length()));
		 pos += with.length()) {
			inout.replace(pos, what.length(), with.data(), with.length());
	}
}

///////////////////////////////////////   DOCUMENT   //////////////////////////////////////////
/*
 Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
 Пример использования:
 Document doc;
 doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
*/
// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
	objects_.emplace_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
	out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
	for (const auto& object : objects_) {
		object->Render(out);
	}
	out << "</svg>"sv;
}

}  // namespace svg
