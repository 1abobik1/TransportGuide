#include <unordered_map>

#include "svg.h"

namespace svg {

    using namespace std::literals;

    // ---------- Object ------------------

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Document ------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        objects_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        // выводим шапку документа
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        // выводим все объекты
        RenderContext ctx(out, 2, 2);
        for (const auto& object : objects_) {
            object->Render(ctx);
        }
        // выводим закрывающий тег
        out << "</svg>"sv;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        // открываем тег
        out << "<polyline "sv;
        // пишем все точки
        out << "points=\""sv;
        auto size = points_.size();
        if (size != 0) {
            out << points_.at(0).x << ","sv << points_.at(0).y;
            for (auto i = 1u; i < size; ++i) {
                out << " " << points_.at(i).x << ","sv << points_.at(i).y;
            }
        }
        out << "\""sv;
        RenderAttrs(out);
        // закрываем тег
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(const std::string& font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(const std::string& font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(const std::string& data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        // открываем тег
        out << "<text"sv;
        // пишем свойства
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" "sv;
        out << "y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" "sv;
        out << "dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if (font_family_) {
            out << " font-family=\""sv << font_family_.value() << "\""sv;
        }
        if (font_weight_) {
            out << " font-weight=\""sv << font_weight_.value() << "\""sv;
        }
        // закрываем тег
        out << ">"sv;
        // пишем текст
        out << EscapeText(data_);
        // закрывающий тег текста
        out << "</text>"sv;
    }

    std::string Text::EscapeText(const std::string& text) {
        // экранирующие значения для спецсимволов
        std::unordered_map<char, std::string> characters = { {34,"&quot;"s},
                                                            {38,"&amp;"s},
                                                            {39,"&apos;"s},
                                                            {60,"&lt;"s},
                                                            {62,"&gt;"s} };
        std::string result;
        // проходим по всем символам строки, если это спецсимвол - экранируем
        for (auto c : text) {
            auto iter = characters.find(c);
            if (iter != characters.end()) {
                result += iter->second;
            }
            else {
                result += c;
            }
        }
        return result;
    }

    std::string TagStrokeLineCap(StrokeLineCap line_cap) {
        std::string result;
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            result = "butt"s;
            break;
        case StrokeLineCap::ROUND:
            result = "round"s;
            break;
        case StrokeLineCap::SQUARE:
            result = "square"s;
            break;
        }
        return result;
    }

    std::string TagStrokeLineJoin(StrokeLineJoin line_join) {
        std::string result;
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            result = "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            result = "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            result = "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            result = "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            result = "round"s;
            break;
        }
        return result;
    }

    std::ostream& operator<<(std::ostream& out, Color color) {
        std::visit(OstreamColorPrinter{ out }, color);
        return out;
    }

    void OstreamColorPrinter::operator()(std::monostate) const {
        out << "none"sv;
    }
    void OstreamColorPrinter::operator()(std::string color) const {
        out << color;
    }
    void OstreamColorPrinter::operator()(Rgb color) const {
        out << "rgb("sv << int(color.red) << ","sv << int(color.green)
            << ","sv << int(color.blue) << ")"sv;
    }
    void OstreamColorPrinter::operator()(Rgba color) const {
        out << "rgba("sv << int(color.red) << ","sv << int(color.green)
            << ","sv << int(color.blue) << ","sv << color.opacity << ")"sv;
    }

}  // namespace svg