#include "svg_icons.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <utility>
#include <wrl/client.h>

namespace
{
struct IconSource
{
    float viewBox;
    const char *path;
};

IconSource GetIconSource(SvgIcon icon)
{
    switch (icon)
    {
    case SvgIcon::Appearance:
        return {12.0f, "M5.99038 10.75C5.34229 10.75 4.73013 10.6252 4.15388 10.3755C3.57754 10.1258 3.07433 9.78592 2.64425 9.35575C2.21408 8.92567 1.87417 8.42167 1.6245 7.84375C1.37483 7.26583 1.25 6.65125 1.25 6C1.25 5.34042 1.37821 4.72167 1.63463 4.14375C1.89104 3.56583 2.23929 3.06312 "
                       "2.67938 2.63562C3.11938 2.20804 3.63413 1.87021 4.22363 1.62213C4.81304 1.37404 5.44171 1.25 6.10962 1.25C6.73462 1.25 7.32692 1.35658 7.8865 1.56975C8.44617 1.78292 8.93767 2.07796 9.361 2.45488C9.78442 2.83179 10.1218 3.27971 10.373 3.79863C10.6243 4.31746 10.75 4.88008 "
                       "10.75 5.4865C10.75 6.35192 10.4928 7.02883 9.97838 7.51725C9.46396 8.00575 8.8045 8.25 8 8.25H7.09425C6.95192 8.25 6.83812 8.29646 6.75287 8.38938C6.66762 8.48238 6.625 8.59233 6.625 8.71925C6.625 8.87375 6.6875 9.03433 6.8125 9.201C6.9375 9.36767 7 9.559 7 9.775C7 10.1051 "
                       "6.90867 10.3501 6.726 10.5101C6.54325 10.67 6.29804 10.75 5.99038 10.75ZM3.69613 6.19613C3.81538 6.07688 3.875 5.92817 3.875 5.75C3.875 5.57183 3.81538 5.42313 3.69613 5.30388C3.57688 5.18463 3.42817 5.125 3.25 5.125C3.07183 5.125 2.92312 5.18463 2.80387 5.30388C2.68462 "
                       "5.42313 2.625 5.57183 2.625 5.75C2.625 5.92817 2.68462 6.07688 2.80387 6.19613C2.92312 6.31538 3.07183 6.375 3.25 6.375C3.42817 6.375 3.57688 6.31538 3.69613 6.19613ZM5.19613 4.19613C5.31538 4.07688 5.375 3.92817 5.375 3.75C5.375 3.57183 5.31538 3.42313 5.19613 "
                       "3.30388C5.07688 3.18462 4.92817 3.125 4.75 3.125C4.57183 3.125 4.42313 3.18462 4.30388 3.30388C4.18463 3.42313 4.125 3.57183 4.125 3.75C4.125 3.92817 4.18463 4.07688 4.30388 4.19613C4.42313 4.31538 4.57183 4.375 4.75 4.375C4.92817 4.375 5.07688 4.31538 5.19613 "
                       "4.19613ZM7.69613 4.19613C7.81538 4.07688 7.875 3.92817 7.875 3.75C7.875 3.57183 7.81538 3.42313 7.69613 3.30388C7.57688 3.18462 7.42817 3.125 7.25 3.125C7.07183 3.125 6.92313 3.18462 6.80388 3.30388C6.68463 3.42313 6.625 3.57183 6.625 3.75C6.625 3.92817 6.68463 4.07688 "
                       "6.80388 4.19613C6.92313 4.31538 7.07183 4.375 7.25 4.375C7.42817 4.375 7.57688 4.31538 7.69613 4.19613ZM9.19613 6.19613C9.31538 6.07688 9.375 5.92817 9.375 5.75C9.375 5.57183 9.31538 5.42313 9.19613 5.30388C9.07688 5.18463 8.92817 5.125 8.75 5.125C8.57183 5.125 8.42313 "
                       "5.18463 8.30388 5.30388C8.18463 5.42313 8.125 5.57183 8.125 5.75C8.125 5.92817 8.18463 6.07688 8.30388 6.19613C8.42313 6.31538 8.57183 6.375 8.75 6.375C8.92817 6.375 9.07688 6.31538 9.19613 6.19613ZM5.99038 10C6.07179 10 6.13542 9.97996 6.18125 9.93988C6.22708 9.89979 6.25 "
                       "9.84483 6.25 9.775C6.25 9.65833 6.1875 9.52725 6.0625 9.38175C5.9375 9.23625 5.875 9.00837 5.875 8.69812C5.875 8.34171 5.99583 8.05292 6.2375 7.83175C6.47917 7.61058 6.775 7.5 7.125 7.5H8C8.5885 7.5 9.06896 7.32758 9.44138 6.98275C9.81379 6.63783 10 6.13908 10 5.4865C10 "
                       "4.475 9.61217 3.64104 8.8365 2.98463C8.06092 2.32821 7.15196 2 6.10962 2C4.96671 2 3.99604 2.3875 3.19762 3.1625C2.39921 3.9375 2 4.88333 2 6C2 7.10833 2.38958 8.05208 3.16875 8.83125C3.94792 9.61042 4.88846 10 5.99038 10Z"};
    case SvgIcon::Keyboard:
        return {12.0f, "M2.15387 9.25C1.90129 9.25 1.6875 9.1625 1.5125 8.9875C1.3375 8.8125 1.25 8.59871 1.25 8.34613V3.65387C1.25 3.40129 1.3375 3.1875 1.5125 3.0125C1.6875 2.8375 1.90129 2.75 2.15387 2.75H9.84613C10.0987 2.75 10.3125 2.8375 10.4875 3.0125C10.6625 3.1875 10.75 3.40129 10.75 "
                       "3.65387V8.34613C10.75 8.59871 10.6625 8.8125 10.4875 8.9875C10.3125 9.1625 10.0987 9.25 9.84613 9.25H2.15387ZM2.15387 8.5H9.84613C9.88463 8.5 9.91988 8.48396 9.95188 8.45188C9.98396 8.41988 10 8.38463 10 8.34613V3.65387C10 3.61537 9.98396 3.58012 9.95188 3.54812C9.91988 "
                       "3.51604 9.88463 3.5 9.84613 3.5H2.15387C2.11537 3.5 2.08013 3.51604 2.04813 3.54812C2.01604 3.58012 2 3.61537 2 3.65387V8.34613C2 8.38463 2.01604 8.41988 2.04813 8.45188C2.08013 8.48396 2.11537 8.5 2.15387 8.5ZM4.05775 7.94225H7.94225V7.05775H4.05775V7.94225ZM2.55775 "
                       "6.44225H3.44225V5.55775H2.55775V6.44225ZM4.05775 6.44225H4.94225V5.55775H4.05775V6.44225ZM5.55775 6.44225H6.44225V5.55775H5.55775V6.44225ZM7.05775 6.44225H7.94225V5.55775H7.05775V6.44225ZM8.55775 6.44225H9.44225V5.55775H8.55775V6.44225ZM2.55775 "
                       "4.94225H3.44225V4.05775H2.55775V4.94225ZM4.05775 4.94225H4.94225V4.05775H4.05775V4.94225ZM5.55775 4.94225H6.44225V4.05775H5.55775V4.94225ZM7.05775 4.94225H7.94225V4.05775H7.05775V4.94225ZM8.55775 4.94225H9.44225V4.05775H8.55775V4.94225Z"};
    case SvgIcon::Shortcut:
        return {
            12.0f,
            "M3.25 10.375C2.79875 10.375 2.41508 10.217 2.099 9.901C1.783 9.58492 1.625 9.20125 1.625 8.75C1.625 8.29875 1.783 7.91508 2.099 7.599C2.41508 7.283 2.79875 7.125 3.25 7.125H4.125V4.875H3.25C2.79875 4.875 2.41508 4.717 2.099 4.401C1.783 4.08492 1.625 3.70125 1.625 3.25C1.625 2.79875 "
            "1.783 2.41508 2.099 2.099C2.41508 1.783 2.79875 1.625 3.25 1.625C3.70125 1.625 4.08492 1.783 4.401 2.099C4.717 2.41508 4.875 2.79875 4.875 3.25V4.125H7.125V3.25C7.125 2.79875 7.283 2.41508 7.599 2.099C7.91508 1.783 8.29875 1.625 8.75 1.625C9.20125 1.625 9.58492 1.783 9.901 "
            "2.099C10.217 2.41508 10.375 2.79875 10.375 3.25C10.375 3.70125 10.217 4.08492 9.901 4.401C9.58492 4.717 9.20125 4.875 8.75 4.875H7.875V7.125H8.75C9.20125 7.125 9.58492 7.283 9.901 7.599C10.217 7.91508 10.375 8.29875 10.375 8.75C10.375 9.20125 10.217 9.58492 9.901 9.901C9.58492 10.217 "
            "9.20125 10.375 8.75 10.375C8.29875 10.375 7.91508 10.217 7.599 9.901C7.283 9.58492 7.125 9.20125 7.125 8.75V7.875H4.875V8.75C4.875 9.20125 4.717 9.58492 4.401 9.901C4.08492 10.217 3.70125 10.375 3.25 10.375ZM3.25 9.625C3.49042 9.625 3.69633 9.53925 3.86775 9.36775C4.03925 9.19633 "
            "4.125 8.99042 4.125 8.75V7.875H3.25C3.00958 7.875 2.80367 7.96075 2.63225 8.13225C2.46075 8.30367 2.375 8.50958 2.375 8.75C2.375 8.99042 2.46075 9.19633 2.63225 9.36775C2.80367 9.53925 3.00958 9.625 3.25 9.625ZM8.75 9.625C8.99042 9.625 9.19633 9.53925 9.36775 9.36775C9.53925 9.19633 "
            "9.625 8.99042 9.625 8.75C9.625 8.50958 9.53925 8.30367 9.36775 8.13225C9.19633 7.96075 8.99042 7.875 8.75 7.875H7.875V8.75C7.875 8.99042 7.96075 9.19633 8.13225 9.36775C8.30367 9.53925 8.50958 9.625 8.75 9.625ZM4.875 7.125H7.125V4.875H4.875V7.125ZM3.25 4.125H4.125V3.25C4.125 3.00958 "
            "4.03925 2.80367 3.86775 2.63225C3.69633 2.46075 3.49042 2.375 3.25 2.375C3.00958 2.375 2.80367 2.46075 2.63225 2.63225C2.46075 2.80367 2.375 3.00958 2.375 3.25C2.375 3.49042 2.46075 3.69633 2.63225 3.86775C2.80367 4.03925 3.00958 4.125 3.25 4.125ZM7.875 4.125H8.75C8.99042 4.125 "
            "9.19633 4.03925 9.36775 3.86775C9.53925 3.69633 9.625 3.49042 9.625 3.25C9.625 3.00958 9.53925 2.80367 9.36775 2.63225C9.19633 2.46075 8.99042 2.375 8.75 2.375C8.50958 2.375 8.30367 2.46075 8.13225 2.63225C7.96075 2.80367 7.875 3.00958 7.875 3.25V4.125Z"};
    case SvgIcon::About:
        return {12.0f, "M5.625 8.375H6.375V5.5H5.625V8.375ZM6.2865 4.52813C6.36475 4.45071 6.40388 4.35479 6.40388 4.24037C6.40388 4.12596 6.36517 4.03004 6.28775 3.95263C6.21033 3.87529 6.11442 3.83662 6 3.83662C5.88558 3.83662 5.78967 3.87529 5.71225 3.95263C5.63483 4.03004 5.59613 4.12596 "
                       "5.59613 4.24037C5.59613 4.35479 5.63525 4.45071 5.7135 4.52813C5.79167 4.60554 5.88717 4.64425 6 4.64425C6.11283 4.64425 6.20833 4.60554 6.2865 4.52813ZM6.00088 10.75C5.34388 10.75 4.72633 10.6253 4.14825 10.376C3.57017 10.1267 3.06733 9.78829 2.63975 9.36088C2.21217 "
                       "8.93346 1.87362 8.43083 1.62412 7.853C1.37471 7.27517 1.25 6.65779 1.25 6.00088C1.25 5.34388 1.37467 4.72633 1.624 4.14825C1.87333 3.57017 2.21171 3.06733 2.63913 2.63975C3.06654 2.21217 3.56917 1.87362 4.147 1.62412C4.72483 1.37471 5.34221 1.25 5.99912 1.25C6.65613 1.25 "
                       "7.27367 1.37467 7.85175 1.624C8.42983 1.87333 8.93267 2.21171 9.36025 2.63913C9.78783 3.06654 10.1264 3.56917 10.3759 4.147C10.6253 4.72483 10.75 5.34221 10.75 5.99912C10.75 6.65613 10.6253 7.27367 10.376 7.85175C10.1267 8.42983 9.78829 8.93267 9.36088 9.36025C8.93346 "
                       "9.78783 8.43083 10.1264 7.853 10.3759C7.27517 10.6253 6.65779 10.75 6.00088 10.75ZM6 10C7.11667 10 8.0625 9.6125 8.8375 8.8375C9.6125 8.0625 10 7.11667 10 6C10 4.88333 9.6125 3.9375 8.8375 3.1625C8.0625 2.3875 7.11667 2 6 2C4.88333 2 3.9375 2.3875 3.1625 3.1625C2.3875 "
                       "3.9375 2 4.88333 2 6C2 7.11667 2.3875 8.0625 3.1625 8.8375C3.9375 9.6125 4.88333 10 6 10Z"};
    case SvgIcon::Arrow:
    default:
        return {24.0f, "M11.2929 4.29289C11.6834 3.90237 12.3166 3.90237 12.7071 4.29289L16.7071 8.29289C17.0976 8.68342 17.0976 9.31658 16.7071 9.70711C16.3166 10.0976 15.6834 10.0976 15.2929 9.70711L12 6.41421L8.70711 9.70711C8.31658 10.0976 7.68342 10.0976 7.29289 9.70711C6.90237 9.31658 "
                       "6.90237 8.68342 7.29289 8.29289L11.2929 4.29289ZM7.29289 14.2929C7.68342 13.9024 8.31658 13.9024 8.70711 14.2929L12 17.5858L15.2929 14.2929C15.6834 13.9024 16.3166 13.9024 16.7071 14.2929C17.0976 14.6834 17.0976 15.3166 16.7071 15.7071L12.7071 19.7071C12.3166 20.0976 "
                       "11.6834 20.0976 11.2929 19.7071L7.29289 15.7071C6.90237 15.3166 6.90237 14.6834 7.29289 14.2929Z"};
    }
}

// Minimal SVG path-data reader: M/L/H/V/C/S/Q/T/Z in absolute and relative form.
// Arcs are not needed by the bundled icons and are rejected.
class PathReader
{
  public:
    explicit PathReader(const char *data) : p_(data)
    {
    }

    bool Build(ID2D1GeometrySink *sink)
    {
        char command = 0;
        while (SkipSeparators())
        {
            if (std::isalpha(static_cast<unsigned char>(*p_)))
            {
                command = *p_++;
            }
            else if (command == 0)
            {
                return false;
            }
            // A bare coordinate pair after M/m is an implicit L/l.
            if (!Execute(command, sink))
            {
                return false;
            }
            if (command == 'M')
            {
                command = 'L';
            }
            else if (command == 'm')
            {
                command = 'l';
            }
        }
        EndFigure(sink, false);
        return true;
    }

  private:
    bool Execute(char command, ID2D1GeometrySink *sink)
    {
        const bool relative = std::islower(static_cast<unsigned char>(command)) != 0;
        const float ox = relative ? current_.x : 0.0f;
        const float oy = relative ? current_.y : 0.0f;
        switch (std::toupper(static_cast<unsigned char>(command)))
        {
        case 'M': {
            float x = 0, y = 0;
            if (!Number(x) || !Number(y))
            {
                return false;
            }
            EndFigure(sink, false);
            current_ = start_ = D2D1::Point2F(ox + x, oy + y);
            sink->BeginFigure(current_, D2D1_FIGURE_BEGIN_FILLED);
            open_ = true;
            lastControl_ = current_;
            return true;
        }
        case 'L': {
            float x = 0, y = 0;
            if (!Number(x) || !Number(y))
            {
                return false;
            }
            return LineTo(sink, D2D1::Point2F(ox + x, oy + y));
        }
        case 'H': {
            float x = 0;
            if (!Number(x))
            {
                return false;
            }
            return LineTo(sink, D2D1::Point2F(ox + x, current_.y));
        }
        case 'V': {
            float y = 0;
            if (!Number(y))
            {
                return false;
            }
            return LineTo(sink, D2D1::Point2F(current_.x, oy + y));
        }
        case 'C': {
            float x1 = 0, y1 = 0, x2 = 0, y2 = 0, x = 0, y = 0;
            if (!Number(x1) || !Number(y1) || !Number(x2) || !Number(y2) || !Number(x) || !Number(y))
            {
                return false;
            }
            return CubicTo(sink, D2D1::Point2F(ox + x1, oy + y1), D2D1::Point2F(ox + x2, oy + y2), D2D1::Point2F(ox + x, oy + y));
        }
        case 'S': {
            float x2 = 0, y2 = 0, x = 0, y = 0;
            if (!Number(x2) || !Number(y2) || !Number(x) || !Number(y))
            {
                return false;
            }
            const D2D1_POINT_2F c1 = D2D1::Point2F(2.0f * current_.x - lastControl_.x, 2.0f * current_.y - lastControl_.y);
            return CubicTo(sink, c1, D2D1::Point2F(ox + x2, oy + y2), D2D1::Point2F(ox + x, oy + y));
        }
        case 'Q': {
            float x1 = 0, y1 = 0, x = 0, y = 0;
            if (!Number(x1) || !Number(y1) || !Number(x) || !Number(y))
            {
                return false;
            }
            return QuadTo(sink, D2D1::Point2F(ox + x1, oy + y1), D2D1::Point2F(ox + x, oy + y));
        }
        case 'T': {
            float x = 0, y = 0;
            if (!Number(x) || !Number(y))
            {
                return false;
            }
            const D2D1_POINT_2F c = D2D1::Point2F(2.0f * current_.x - lastControl_.x, 2.0f * current_.y - lastControl_.y);
            return QuadTo(sink, c, D2D1::Point2F(ox + x, oy + y));
        }
        case 'Z':
            EndFigure(sink, true);
            current_ = lastControl_ = start_;
            return true;
        default:
            return false;
        }
    }

    bool EnsureOpen(ID2D1GeometrySink *sink)
    {
        if (!open_)
        {
            sink->BeginFigure(current_, D2D1_FIGURE_BEGIN_FILLED);
            start_ = current_;
            open_ = true;
        }
        return true;
    }

    bool LineTo(ID2D1GeometrySink *sink, D2D1_POINT_2F point)
    {
        EnsureOpen(sink);
        sink->AddLine(point);
        current_ = lastControl_ = point;
        return true;
    }

    bool CubicTo(ID2D1GeometrySink *sink, D2D1_POINT_2F c1, D2D1_POINT_2F c2, D2D1_POINT_2F end)
    {
        EnsureOpen(sink);
        sink->AddBezier(D2D1::BezierSegment(c1, c2, end));
        lastControl_ = c2;
        current_ = end;
        return true;
    }

    bool QuadTo(ID2D1GeometrySink *sink, D2D1_POINT_2F control, D2D1_POINT_2F end)
    {
        EnsureOpen(sink);
        sink->AddQuadraticBezier(D2D1::QuadraticBezierSegment(control, end));
        lastControl_ = control;
        current_ = end;
        return true;
    }

    void EndFigure(ID2D1GeometrySink *sink, bool closed)
    {
        if (open_)
        {
            sink->EndFigure(closed ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
            open_ = false;
        }
    }

    bool SkipSeparators()
    {
        while (*p_ && (std::isspace(static_cast<unsigned char>(*p_)) || *p_ == ','))
        {
            ++p_;
        }
        return *p_ != '\0';
    }

    bool Number(float &out)
    {
        SkipSeparators();
        char *end = nullptr;
        const double value = std::strtod(p_, &end);
        if (end == p_)
        {
            return false;
        }
        p_ = end;
        out = static_cast<float>(value);
        return true;
    }

    const char *p_ = nullptr;
    D2D1_POINT_2F current_ = {0.0f, 0.0f};
    D2D1_POINT_2F start_ = {0.0f, 0.0f};
    D2D1_POINT_2F lastControl_ = {0.0f, 0.0f};
    bool open_ = false;
};

struct GeometryCacheEntry
{
    Microsoft::WRL::ComPtr<ID2D1Factory> factory;
    Microsoft::WRL::ComPtr<ID2D1PathGeometry> geometry;
};

// Geometries are device-independent, so one per (factory, icon) is enough. The
// entry keeps its factory alive so a recycled pointer can never alias a stale entry.
ID2D1PathGeometry *GetGeometry(ID2D1Factory *factory, SvgIcon icon)
{
    static std::map<std::pair<ID2D1Factory *, int>, GeometryCacheEntry> cache;
    const auto key = std::make_pair(factory, static_cast<int>(icon));
    auto it = cache.find(key);
    if (it != cache.end())
    {
        return it->second.geometry.Get();
    }

    GeometryCacheEntry entry;
    entry.factory = factory;
    if (FAILED(factory->CreatePathGeometry(entry.geometry.GetAddressOf())))
    {
        return nullptr;
    }
    Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
    if (FAILED(entry.geometry->Open(sink.GetAddressOf())))
    {
        return nullptr;
    }
    sink->SetFillMode(D2D1_FILL_MODE_WINDING);
    PathReader reader(GetIconSource(icon).path);
    const bool ok = reader.Build(sink.Get());
    if (FAILED(sink->Close()) || !ok)
    {
        return nullptr;
    }
    return cache.emplace(key, std::move(entry)).first->second.geometry.Get();
}
} // namespace

void DrawSvgIcon(ID2D1RenderTarget *target, ID2D1Brush *brush, SvgIcon icon, const D2D1_RECT_F &rect)
{
    if (!target || !brush)
    {
        return;
    }
    Microsoft::WRL::ComPtr<ID2D1Factory> factory;
    target->GetFactory(factory.GetAddressOf());
    if (!factory)
    {
        return;
    }
    ID2D1PathGeometry *geometry = GetGeometry(factory.Get(), icon);
    if (!geometry)
    {
        return;
    }

    const float size = (std::min)(rect.right - rect.left, rect.bottom - rect.top);
    const float scale = size / GetIconSource(icon).viewBox;
    const float left = rect.left + (rect.right - rect.left - size) * 0.5f;
    const float top = rect.top + (rect.bottom - rect.top - size) * 0.5f;

    D2D1_MATRIX_3X2_F previous;
    target->GetTransform(&previous);
    target->SetTransform(D2D1::Matrix3x2F::Scale(scale, scale) * D2D1::Matrix3x2F::Translation(left, top) * previous);
    target->FillGeometry(geometry, brush);
    target->SetTransform(previous);
}
