#include <raylib.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>

#define exprtk_lean_and_mean_numeric_only
#include "exprtk.hpp"

class PlotInfo;
void DrawPlotGrid(const PlotInfo& plot);
void DrawPlotAxis(const PlotInfo& plot);

template<typename T> 
struct Vec2 {
    T x, y;

    operator Vector2() const {
        return Vector2{
            static_cast<float>(x),
            static_cast<float>(y)
        };
    }
};

class PlotInfo {
private:
    Vec2<float> coordinates;

    float sizeUnit;
    int sizePixel;
    float pixelPerUnit;

public:
    PlotInfo(
        Vec2<float> coordinates,
        float sizeUnit,
        int sizePixel
    );

    void Move(Vec2<float> distance);
    void Zoom(float coefficient);

    friend void DrawPlotGrid(const PlotInfo& plot);
    friend void DrawPlotAxis(const PlotInfo& plot);
    template<typename F>
    friend void DrawPlotFunction(const PlotInfo& plot, F function, Color color);
    friend Vec2<float> UnitToScreenVector(const PlotInfo& plot, Vec2<float> vector);
};

PlotInfo::PlotInfo(
        Vec2<float> coordinates,
        float sizeUnit,
        int sizePixel
    )
    : coordinates(coordinates),
      sizeUnit(sizeUnit),
      sizePixel(sizePixel),
      pixelPerUnit(sizePixel/sizeUnit)
{
}

class Expression {
private:
    exprtk::symbol_table<float> symbol_table;
    exprtk::expression<float>   expression;
    exprtk::parser<float>       parser;
    float var;
public:
    bool expressionInit(const std::string expression_string) {
        symbol_table = exprtk::symbol_table<float>(exprtk::symbol_table<float>::e_immutable);
        symbol_table.add_variable("x", var);
        symbol_table.add_constants();
        
        expression.register_symbol_table(symbol_table);

        exprtk::parser<float> parser;

        if (!parser.compile(expression_string, expression)) {
            printf("Error: %s\n", parser.error().c_str());
            return false;
        }

        return true;
    }

    float evaluate(float x) {
        var = x;
        return expression.value();
    }
};

void PlotInfo::Move(Vec2<float> distance) {
    coordinates.x += distance.x;
    coordinates.y += distance.y;
}

void PlotInfo::Zoom(float coefficient) {
    if (sizeUnit <= 1) {
        return;
    }
    sizeUnit += coefficient;
    pixelPerUnit = sizePixel/sizeUnit;
}

void DrawPlotGrid(const PlotInfo& plot) {
    Vec2<float> lineOffset = {
        (std::ceilf(plot.coordinates.x) - plot.coordinates.x) * plot.pixelPerUnit,
        (plot.coordinates.y - std::floorf(plot.coordinates.y)) * plot.pixelPerUnit
    };

    int countLines = std::floorf(plot.sizeUnit);
    
    Vec2<float> lineEnd = {
        static_cast<float>(plot.sizePixel),
        static_cast<float>(plot.sizePixel)
    };

    // Horizontal lines
    for (int i{}; i < countLines; i++) {
        float lineY = lineOffset.y + plot.pixelPerUnit * i;

        DrawLineEx(
            {
                0,
                lineY,
            }, {
                lineEnd.x,
                lineY,
            },
            1, GRAY
        );

        DrawText(
            std::to_string(static_cast<int>(std::floorf(plot.coordinates.y+plot.sizeUnit))-i).c_str(),
            0, lineY, 15, WHITE
        );
    }

    // Vertical lines
    for (int i{}; i < countLines; i++) {
        float lineX = lineOffset.x + plot.pixelPerUnit * i;

        DrawLineEx(
            {
                lineX,
                0,
            }, {
                lineX,
                lineEnd.y,
            },
            1, GRAY
        );

        DrawText(
            std::to_string(static_cast<int>(std::ceilf(plot.coordinates.x))+i).c_str(),
            lineX, lineEnd.y-13, 15, WHITE
        );
    }
    
}

void DrawPlotAxis(const PlotInfo& plot) {
    // Horizontal Axis
    if (plot.coordinates.y <= 0 && plot.coordinates.y + plot.sizeUnit >= 0) {
        float lineY = (plot.sizeUnit + plot.coordinates.y) * plot.pixelPerUnit;

        DrawLineEx(
            {
                0,
                lineY
            }, {
                static_cast<float>(plot.sizePixel),
                lineY
            },
            4, GREEN
        );
    }

    // Vertiacal Axis
    if (plot.coordinates.x <= 0 && plot.coordinates.x + plot.sizeUnit >= 0) {
        float lineX = -plot.coordinates.x * plot.pixelPerUnit;

        DrawLineEx(
            {
                lineX,
                0
            }, {
                lineX,
                static_cast<float>(plot.sizePixel)
            },
            4, RED
        );
    }
}

Vec2<float> UnitToScreenVector(const PlotInfo& plot, Vec2<float> vector) {
    return {
        .x = (vector.x - plot.coordinates.x) * plot.pixelPerUnit,
        .y = (plot.coordinates.y + plot.sizeUnit - vector.y) * plot.pixelPerUnit
    };
}

bool IsValidCoordinate(float value, float low, float high) {
    return value >= low && value <= high;
}

template<typename F>
void DrawPlotFunction(const PlotInfo& plot, F function, Color color) {
    Vec2<float> plotEndUnit = {
        plot.coordinates.x + plot.sizeUnit,
        plot.coordinates.y + plot.sizeUnit,
    };
    
    float dx = plot.sizeUnit / plot.sizePixel / 4.f;

    float x = plot.coordinates.x;
    float y = function(x);
    Vec2<float> lineBegin = UnitToScreenVector(plot, {x, y});

    x += dx;

    bool interrupted{};
    for (; x < plotEndUnit.x-dx; x += dx) {    
        float xNext = x + dx;
        float yNext = function(xNext);
        
        if (!std::isfinite(y) || !std::isfinite(yNext)) {
            y = yNext;
            interrupted = true;
            continue;
        }

        if ((y <= plot.coordinates.y && yNext >= plotEndUnit.y) ||
            (yNext <= plot.coordinates.y && y >= plotEndUnit.y)){
            y = yNext;
            interrupted = true;
            continue;
        }
        
        if (interrupted) {
            y = std::clamp(y, plot.coordinates.y-1, plotEndUnit.y+1);
            lineBegin = UnitToScreenVector(plot, {x, y});
            interrupted = false;
        }

        y = yNext;
        
        yNext = std::clamp(yNext, plot.coordinates.y-1, plotEndUnit.y+1);

        Vec2<float> lineEnd = UnitToScreenVector(plot, {xNext, yNext});

        DrawLineEx(lineBegin, lineEnd, 4, color);
        lineBegin = lineEnd;
    }
}

int main(int argc, char const *argv[]) {
    if (argc == 1) {
        std::cerr << "Error: Need to provide expressions\n";
        return 1;
    }

    Color colorPalete[] = {
        YELLOW, VIOLET, BLUE, PURPLE, BROWN, LIME, SKYBLUE, ORANGE, GOLD, GRAY, WHITE
    };

    std::srand(time(nullptr));
    
    Expression expressions [argc-1];
    std::function<float(float)> functions[argc-1];
    Color colors[argc-1];

    for (int i{1}; i < argc; ++i) {
        auto& e = expressions[i-1];

        if (!e.expressionInit(argv[i])) {
            return 1;
        }

        functions[i-1] = [&e](float x) -> float {
            return e.evaluate(x);
        };

        colors[i-1] = colorPalete[(rand() % std::size(colorPalete))];
    }

    SetTraceLogLevel(LOG_ERROR);
    InitWindow(1000, 1000, "Graph");
    SetTargetFPS(60);
    
    PlotInfo plot({-10.f, -10.f}, 20.f, 1000);
    const float speed = 0.5;

    while (!WindowShouldClose()) {
        {
            float distance = GetTime() * speed;

            if (IsKeyDown(KEY_UP)) {
                plot.Move({0, speed});
            }
            if (IsKeyDown(KEY_DOWN)) {
                plot.Move({0, -speed});
            }
            if (IsKeyDown(KEY_RIGHT)) {
                plot.Move({speed, 0});
            }
            if (IsKeyDown(KEY_LEFT)) {
                plot.Move({-speed, 0});
            }
            if (IsKeyPressed(KEY_EQUAL)) {
                plot.Zoom(1);
            }
            if (IsKeyPressed(KEY_MINUS)) {
                plot.Zoom(-1);
            }
        }

        BeginDrawing();
        {
            ClearBackground(BLACK);

            DrawPlotGrid(plot);
            DrawPlotAxis(plot);
            
            for (int i = 0; i < argc-1; ++i) {
                DrawPlotFunction(plot, functions[i], colors[i]);
            }

            DrawFPS(0, 0);
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
