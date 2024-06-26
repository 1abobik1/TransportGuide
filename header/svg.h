#pragma once

#include <iostream>
#include <ostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    /*
     * ��������������� ���������, �������� �������� ��� ������ SVG-��������� � ���������.
     * ������ ������ �� ����� ������, ������� �������� � ��� ������� ��� ������ ��������
     */
    struct RenderContext {
        RenderContext(std::ostream& out)
            : out(out) {
        }
        RenderContext(std::ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }
        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }
        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }
        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    // �������� ����

    struct Rgb {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double o) : red(r), green(g), blue(b), opacity(o) {}
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
    inline const Color NoneColor{};
    struct OstreamColorPrinter {
        std::ostream& out;
        void operator()(std::monostate) const;
        void operator()(std::string) const;
        void operator()(Rgb) const;
        void operator()(Rgba) const;
    };
    std::ostream& operator<<(std::ostream& out, Color color);

    // ��������������� ���� ��� PathProps

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };
    std::string TagStrokeLineCap(StrokeLineCap line_cap);
    inline std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        out << TagStrokeLineCap(line_cap);
        return out;
    }

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };
    std::string TagStrokeLineJoin(StrokeLineJoin line_join);
    inline std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        out << TagStrokeLineJoin(line_join);
        return out;
    }

    // ����� PathProps ��������� ������� � ������ ��������.
    template <typename Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color color);
        Owner& SetStrokeColor(Color color);
        Owner& SetStrokeWidth(double width);
        Owner& SetStrokeLineCap(StrokeLineCap line_cap);
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join);
    protected:
        ~PathProps() = default;
        void RenderAttrs(std::ostream& out) const;
    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    /*
     * ����������� ������� ����� Object ������ ��� ���������������� ��������
     * ���������� ����� SVG-���������
     * ��������� ������� "��������� �����" ��� ������ ����������� ����
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;
    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    // ����� ObjectContainer - ��������� ��������� ��� ������� � ���������� SVG-��������.
    class ObjectContainer {
    public:
        // ����� Add ��������� � svg-�������� ����� ������-��������� svg::Object.
        template <class Obj>
        void Add(Obj obj) {
            // �������� ����� AddPtr ���������������� ����������
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        virtual ~ObjectContainer() = default;
    };

    // ����� Drawable - ��������� ��������� ��������� �� ������� ObjectContainer
    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    // ����� Document - ��������� SVG-�������� � ������� � �����
    class Document final : public ObjectContainer {
    public:
        // ��������� � svg-�������� ������-��������� svg::Object
        void AddPtr(std::unique_ptr<Object>&& obj) override;
        // ������� � ostream svg-������������� ���������
        void Render(std::ostream& out) const;
    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

    // ����� Circle ���������� ������� <circle> ��� ����������� �����
    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);
    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };

    // ����� Polyline ���������� ������� <polyline> ��� ����������� ������� �����
    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        // ��������� ��������� ������� � ������� �����
        Polyline& AddPoint(Point point);
    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points_;
    };

    // ����� Text ���������� ������� <text> ��� ����������� ������
    class Text final : public Object, public PathProps<Text> {
    public:
        // ����� ���������� ������� ����� (�������� x � y)
        Text& SetPosition(Point pos);
        // ����� �������� ������������ ������� ����� (�������� dx, dy)
        Text& SetOffset(Point offset);
        // ����� ������� ������ (������� font-size)
        Text& SetFontSize(uint32_t size);
        // ����� �������� ������ (������� font-family)
        Text& SetFontFamily(const std::string& font_family);
        // ����� ������� ������ (������� font-weight)
        Text& SetFontWeight(const std::string& font_weight);
        // ����� ��������� ���������� ������� (������������ ������ ���� text)
        Text& SetData(const std::string& data);
    private:
        void RenderObject(const RenderContext& context) const override;
        // ���������� ������ � ��������������� ���������� ���������
        static std::string EscapeText(const std::string& text);

        Point pos_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::optional<std::string> font_family_;
        std::optional<std::string> font_weight_;
        std::string data_;
    };

    template<typename Owner>
    Owner& PathProps<Owner>::SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    template<typename Owner>
    Owner& PathProps<Owner>::SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    template<typename Owner>
    Owner& PathProps<Owner>::SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }

    template<typename Owner>
    Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
        stroke_line_cap_ = line_cap;
        return AsOwner();
    }

    template<typename Owner>
    Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
        stroke_line_join_ = line_join;
        return AsOwner();
    }

    template<typename Owner>
    void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }

        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }

        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }

        if (stroke_line_cap_) {
            out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
        }

        if (stroke_line_join_) {
            out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
        }
    }

}  // namespace svg