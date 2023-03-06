#include "map_renderer.h"
#include <memory>
#include <set>

namespace renderer {

bool IsZero(double value) {
	return std::abs(value) < EPSILON;
}

svg::Color RenderSettings::RenderColor (int route_index) const {
	if (color_palette.empty()) return svg::NoneColor;
	return color_palette[route_index % color_palette.size()];
}

MapRenderer::MapRenderer(std::deque<domain::Bus> all_buses, RenderSettings rs)
	: all_buses_(std::move(all_buses))
	, rs_(std::move(rs)) {
}

void MapRenderer::SetRenderer() {
	MakeRouteToGeoCoordsAndStop();
	MakeAllGeoCoordsArr();
	MakeRoutesWithPlainCoords();
	RenderMap();
}

void MapRenderer::OutputRenderedMap (std::ostream& out) {
	RenderMap().Render(out);
}

svg::Document MapRenderer::GetRenderedMap () const {
	return RenderMap();
}

void MapRenderer::MakeRouteToGeoCoordsAndStop() {
	RouteToGeoCoords route_to_gcoords;
	for (auto it = all_buses_.begin(); it != all_buses_.end(); ++it) {
		route_to_gcoords.bus = &*it;
		std::vector<geo::Coordinates> tmp_coords;
		std::vector<domain::Stop*> tmp_stops;

		// adding stops from begin to last for both types routes
		for (long unsigned int i = 0; i < it->route_.size(); ++i) {
			tmp_stops.push_back(it->route_[i]);
			tmp_coords.push_back({it->route_[i]->coordinates.lat,
								  it->route_[i]->coordinates.lng});
		}

		// adding stops from last to begin for non-ring route
		if (it->route_type == domain::RouteType::Line) {
			route_to_gcoords.is_roundtrip = false;
			for (int i = it->route_.size(); i > 1; --i) {
				tmp_stops.push_back(it->route_[i - 2]);
				tmp_coords.push_back({it->route_[i - 2]->coordinates.lat,
									  it->route_[i - 2]->coordinates.lng});
			}
		} else {
			route_to_gcoords.is_roundtrip = true;
		}
		route_to_gcoords.stops = std::move(tmp_stops);
		route_to_gcoords.geo_coords = std::move(tmp_coords);
		routes_to_gcoords_.push_back(route_to_gcoords);
	}
}

svg::Text MapRenderer::MakeBusName(const RouteSVG& route_svg, int pos) const {
	const svg::Text bus_name =	//
			svg::Text()
			.SetFontFamily("Verdana"s)
			.SetFontSize(rs_.bus_label_font_size)
			.SetFontWeight("bold"s)
			.SetPosition(route_svg.route[pos].plane_coord)
			.SetData(route_svg.bus->bus_name)
			.SetOffset(rs_.bus_label_offset);
	return bus_name;
}

svg::Text MapRenderer::MakeStopName(const StopSVG& stop) const {
	const svg::Text stop_name =  //
			svg::Text()
			.SetFillColor("black"s)
			.SetFontFamily("Verdana"s)
			.SetFontSize(rs_.stop_label_font_size)
			.SetPosition(stop.plane_coord)
			.SetData(stop.stop->stop_name)
			.SetOffset(rs_.stop_label_offset);
	return stop_name;
}

svg::Text MapRenderer::GetExtraForName(const svg::Text& name) const {
	const svg::Text extra_name = //
			svg::Text{name}
			.SetStrokeColor(rs_.underlayer_color)
			.SetFillColor(rs_.underlayer_color)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeWidth(rs_.uderlayer_width);
	return extra_name;
}

MapRenderer::DrawablePtrs MapRenderer::MakeRoutesSVG() const {
	std::vector<std::unique_ptr<svg::Drawable>> routes;
	for (long unsigned int index_route = 0; index_route < routes_svg_.size(); ++index_route) {
		if (routes_svg_[index_route].route.empty()) continue;
		Route route (routes_svg_[index_route].route,
					 rs_.RenderColor(index_route),
					 rs_.line_width);
		routes.emplace_back(std::make_unique<Route>(route));
	}
	return routes;
}

MapRenderer::DrawablePtrs MapRenderer::MakeStopsCircles() const {
	std::vector<std::unique_ptr<svg::Drawable>> stop_circles;
	for (const auto& stop : all_stops_svg_) {
		StopCircle stop_circle(stop.plane_coord, rs_.stop_radius);
		stop_circles.emplace_back(std::make_unique<StopCircle>(stop_circle));
	}
	return stop_circles;
}

svg::Document MapRenderer::RenderMap() const {

	svg::Document map;
	DrawPicture(std::move(MakeRoutesSVG()), map); // draw routes

	for (long unsigned int index_route = 0; index_route < routes_svg_.size(); ++index_route) {
		if (routes_svg_[index_route].route.empty()) continue;

		// adding bus name to begin route for all routes
		const svg::Text bus_name = std::move(MakeBusName(routes_svg_[index_route], 0));
		map.Add(std::move(GetExtraForName(bus_name)));
		map.Add(svg::Text{bus_name}.SetFillColor(rs_.RenderColor(index_route)));

		// adding bus name to end of route for non-ring routes
		if (routes_svg_[index_route].bus->route_type == domain::RouteType::Line) {
			int last_stop = routes_svg_[index_route].route.size() / 2;

			// check that begin stop in not end stop for non-ring routes
			if (routes_svg_[index_route].route[last_stop].stop->stop_name !=
				routes_svg_[index_route].route[0].stop->stop_name ) {
				const svg::Text bus_name = std::move(MakeBusName(routes_svg_[index_route], last_stop));
				map.Add(std::move(GetExtraForName(bus_name)));
				map.Add(svg::Text{bus_name}.SetFillColor(rs_.RenderColor(index_route)));
			}
		}
	}

	// draw stop circles
	DrawPicture(std::move(MakeStopsCircles()), map);

	// adding all stop names
	for (const auto& stop : all_stops_svg_) {
		const svg::Text stop_name = std::move(MakeStopName(stop));
		map.Add(std::move(GetExtraForName(stop_name)));
		map.Add(svg::Text{stop_name}.SetFillColor("black"s));
	}
	return map;
}

void MapRenderer::MakeAllGeoCoordsArr() {
	for (const auto& route_to_gcoord : routes_to_gcoords_) {
		geo_coords_.insert(geo_coords_.end(),
						   route_to_gcoord.geo_coords.begin(),
						   route_to_gcoord.geo_coords.end());
	}
}

void MapRenderer::SortingAndMakeUniqueStopsSVG () {
	auto ascending_name = [] (const StopSVG& lhs, const StopSVG& rhs) {
		return lhs.stop->stop_name < rhs.stop->stop_name;
	};
	std::sort(all_stops_svg_.begin(), all_stops_svg_.end(), ascending_name);

	auto is_equal = [] (const StopSVG& lhs, const StopSVG& rhs) {
		return lhs.stop == rhs.stop;
	};
	auto last = std::unique(all_stops_svg_.begin(), all_stops_svg_.end(), is_equal);
	all_stops_svg_.erase(last, all_stops_svg_.end());
}

RouteSVG MapRenderer::MakeRoutesBaseSVG(int route_index, const SphereProjector& proj) {
	RouteSVG output;
	StopSVG stop_svg;
	output.bus = routes_to_gcoords_[route_index].bus;
	for (long unsigned int j = 0; j < routes_to_gcoords_[route_index].geo_coords.size(); ++j) {
		if (routes_to_gcoords_[route_index].stops.empty()) {
			continue;
		}
		stop_svg.stop = routes_to_gcoords_[route_index].stops[j];
		stop_svg.plane_coord = proj(routes_to_gcoords_[route_index].geo_coords[j]);
		output.route.push_back({stop_svg.stop, stop_svg.plane_coord});
		all_stops_svg_.push_back(std::move(stop_svg));
	}
	SortingAndMakeUniqueStopsSVG ();

	return output;
}

void MapRenderer::MakeRoutesWithPlainCoords () {
	const SphereProjector proj {
		geo_coords_.begin(), geo_coords_.end(), rs_.width, rs_.height, rs_.padding
	};
	int routes_count = all_buses_.size();
	for (int route_index = 0; route_index < routes_count; ++route_index) {
		routes_svg_.push_back(std::move(MakeRoutesBaseSVG(route_index, proj)));
	}
}

StopCircle::StopCircle(svg::Point center, double radius)
	: center(center)
	, radius(radius){
}

Route::Route(std::vector<StopSVG> route, svg::Color stroke_color, double stroke_width_)
	: route_(std::move(route))
	, stroke_color_(std::move(stroke_color))
	, stroke_width_(std::move(stroke_width_)) {
}

svg::Polyline Route::MakeRoutePolyline (const std::vector<StopSVG>& stops) const {
	svg::Polyline polyline;
	for (const StopSVG& stop : stops) {
		polyline.AddPoint(stop.plane_coord);
	}
	return polyline;
}

} // namespace renderer
