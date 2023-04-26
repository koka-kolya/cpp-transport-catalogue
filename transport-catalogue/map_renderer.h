#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <unordered_map>

namespace renderer {
using namespace std::literals;

inline const double EPSILON = 1e-6;

bool IsZero(double value);

struct RenderSettings {
	RenderSettings() = default;
	double width = 0;
	double height = 0;
	double padding = 0;
	double line_width = 0;
	double stop_radius = 0;
	double bus_label_font_size = 0;
	svg::Point bus_label_offset {.0, .0};
	double stop_label_font_size = 0;
	svg::Point stop_label_offset {.0, .0};
    svg::Color underlayer_color {};
    double underlayer_width = .0;
	std::vector<svg::Color> color_palette {};
	svg::Color RenderColor (int route_index) const;
};

struct StopSVG {
	domain::Stop* stop;
	svg::Point plane_coord;
};

struct RouteSVG {
	domain::Bus* bus;
	std::vector<StopSVG> route;
	svg::Color color;
};

struct RouteToGeoCoords {
	domain::Bus* bus;
	std::vector<domain::Stop*> stops;
	std::vector<geo::Coordinates> geo_coords;
	bool is_roundtrip;
};

class SphereProjector {
public:
	// points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
	template <typename PointInputIt>
	SphereProjector(PointInputIt points_begin, PointInputIt points_end,
					double width, double height, double padding)
		: padding_(padding) //
	{
		// Если точки поверхности сферы не заданы, вычислять нечего
		if (points_begin == points_end) {
			return;
		}

		// Находим точки с минимальной и максимальной долготой
		const auto [left_it, right_it] = std::minmax_element(
			points_begin, points_end,
			[](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
		min_lon_ = left_it->lng;
		const double max_lon = right_it->lng;

		// Находим точки с минимальной и максимальной широтой
		const auto [bottom_it, top_it] = std::minmax_element(
			points_begin, points_end,
			[](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
		const double min_lat = bottom_it->lat;
		max_lat_ = top_it->lat;

		// Вычисляем коэффициент масштабирования вдоль координаты x
		std::optional<double> width_zoom;
		if (!IsZero(max_lon - min_lon_)) {
			width_zoom = (width - 2 * padding) / (max_lon - min_lon_);
		}

		// Вычисляем коэффициент масштабирования вдоль координаты y
		std::optional<double> height_zoom;
		if (!IsZero(max_lat_ - min_lat)) {
			height_zoom = (height - 2 * padding) / (max_lat_ - min_lat);
		}

		if (width_zoom && height_zoom) {
			// Коэффициенты масштабирования по ширине и высоте ненулевые,
			// берём минимальный из них
			zoom_coeff_ = std::min(*width_zoom, *height_zoom);
		} else if (width_zoom) {
			// Коэффициент масштабирования по ширине ненулевой, используем его
			zoom_coeff_ = *width_zoom;
		} else if (height_zoom) {
			// Коэффициент масштабирования по высоте ненулевой, используем его
			zoom_coeff_ = *height_zoom;
		}
	}

	// Проецирует широту и долготу в координаты внутри SVG-изображения
	svg::Point operator()(geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}

private:
	double padding_;
	double min_lon_ = 0;
	double max_lat_ = 0;
	double zoom_coeff_ = 0;
};

class MapRenderer {
	using BusToGeoCoords = std::unordered_map<domain::Bus*, std::vector<geo::Coordinates>>;
	using Points = std::vector<svg::Point>;
	using GeoCoords = std::vector<geo::Coordinates>;
	using DrawablePtrs = std::vector<std::unique_ptr<svg::Drawable>>;
public:
	MapRenderer();
    MapRenderer(std::deque<domain::Bus> all_buses);

	void SetRenderer();
    void SetRenderSettings(const RenderSettings& rs);
	void OutputRenderedMap (std::ostream& out);
	svg::Document GetRenderedMap () const;

private:
	svg::Document rendered_map_{};
	std::deque<domain::Bus> all_buses_;
	std::vector<StopSVG> all_stops_svg_;
	std::vector<RouteToGeoCoords> routes_to_gcoords_;
	GeoCoords geo_coords_ {};
	std::vector<RouteSVG> routes_svg_;
	RenderSettings rs_;

	// basic rendering methods
	void MakeRouteToGeoCoordsAndStop();
	void MakeAllGeoCoordsArr();
	void MakeRoutesWithPlainCoords();
	svg::Document RenderMap() const;

	// auxiliary methods
	std::deque<svg::Polyline> MakePolylines() const;
	void MakeStopSVG();
	void SortingAndMakeUniqueStopsSVG ();
	RouteSVG MakeRoutesBaseSVG(int route_index, const SphereProjector& proj);
	DrawablePtrs MakeRoutesSVG() const;
	DrawablePtrs MakeStopsCircles() const;
	svg::Text MakeBusName(const RouteSVG& route_svg, int pos) const;
	svg::Text GetExtraForName(const svg::Text& bus_name) const;
	svg::Text MakeStopName(const StopSVG& stop) const;
	void AddBusesNameToMap ();
	void AddingStopNamesToMap();

	template <typename DrawableIterator>
	void DrawPicture(DrawableIterator begin, DrawableIterator end, svg::ObjectContainer& target) const;
	template <typename Container>
	void DrawPicture(const Container& container, svg::ObjectContainer& target) const;
};

class Route : public svg::Drawable {
public:

	Route(std::vector<StopSVG> routes, svg::Color stroke_color, double stroke_width_);
	void Draw(svg::ObjectContainer& container) const override {
		if (route_.empty()) return;
		container.Add(MakeRoutePolyline(route_)
					  .SetFillColor(svg::NoneColor)
					  .SetStrokeColor(stroke_color_)
					  .SetStrokeWidth(stroke_width_)
					  .SetStrokeLineCap(stroke_line_cap_)
					  .SetStrokeLineJoin(stroke_line_join_));
	}

private:
	std::vector<StopSVG> route_;
	svg::Color stroke_color_;
	double stroke_width_;
	svg::StrokeLineCap stroke_line_cap_ = svg::StrokeLineCap::ROUND;
	svg::StrokeLineJoin stroke_line_join_ = svg::StrokeLineJoin::ROUND;
	svg::Polyline MakeRoutePolyline (const std::vector<StopSVG>& stops) const;
};

template <typename DrawableIterator>
void MapRenderer::DrawPicture(DrawableIterator begin,
							  DrawableIterator end,
							  svg::ObjectContainer& target) const {
	for (auto it = begin; it != end; ++it) {
		(*it)->Draw(target);
	}
}

class StopCircle : public svg::Drawable {
public:
	StopCircle(svg::Point center, double radius);
	void Draw(svg::ObjectContainer& container) const override {
		container.Add(svg::Circle().SetCenter(center).SetRadius(radius).SetFillColor(fill));
	}
private:
	svg::Point center;
	double radius;
	svg::Color fill = "white"s;
};

template <typename Container>
void MapRenderer::DrawPicture(const Container& container, svg::ObjectContainer& target) const {
	using namespace std;
	DrawPicture(begin(container), end(container), target);
}

}  // namespace renderer
