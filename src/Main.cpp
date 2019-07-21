#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <cmath>
#include <string>
#include <iostream>

using std::cout, std::endl;

// meta
const std::string appName { "Grapher" };
const float maxFramesPerSecond { 60.0f };
const sf::Vector2u defaultWindowSize { 2048, 1536 };

// origin
const sf::Vector2f defaultOriginPos { (float)defaultWindowSize.x / 2.0f,
                                      (float)defaultWindowSize.y / 2.0f };

// zooming
const float defaultPixelsPerUnit { 100.0f };
const float minPixelsPerUnit = 10.0f;
const float maxPixelsPerUnit = 1000.0f;
const float zoomingSpeed = 1.0f;

// dragging
const float draggingSpeedMultiplicator = 8.0f;
const float draggingSpeedDecayTimeMultiplicator = 10.0f;

// grid
const float gridUnitInterval = 1.0f;
const float subGridUnitInterval = 0.25f;

// colors
const sf::Color backgroundColor { 255, 255, 255 };
const sf::Color axisColor { 0, 0, 0 };
const sf::Color gridColor { 200, 200, 200 };
const sf::Color subGridColor { 235, 235, 235 };


int main() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(defaultWindowSize.x, defaultWindowSize.y),
                            appName, sf::Style::Close, settings);

    sf::Vector2f windowSize(window.getSize());
    float pixelsPerUnit = defaultPixelsPerUnit;
    sf::Vector2f originPos = defaultOriginPos;

    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
    bool isDragging = false;  
    sf::Vector2f draggingSpeed { 0.0f, 0.0f }; 

    sf::Font font;
    font.loadFromFile("res/Verdana.ttf");

    sf::Event event;    
    sf::Clock clock;
    const float timeBetweenFrames = 1.0 / maxFramesPerSecond;

    while(window.isOpen()) {

        while(window.pollEvent(event)) {
            switch(event.type) {
                case sf::Event::Closed:
                    window.close();
                    break;

                case sf::Event::KeyPressed:
                    switch(event.key.code) {
                        case sf::Keyboard::Space:
                            originPos = defaultOriginPos;
                            break;

                        default:
                            break;
                    }
                    break;

                case sf::Event::MouseButtonPressed:
                    switch(event.mouseButton.button) {
                        case sf::Mouse::Left:
                            isDragging = true;
                            break;
                        case sf::Mouse::Right:
                            break;
                        default:
                            break;
                    }
                    break;

                case sf::Event::MouseButtonReleased:
                    switch(event.mouseButton.button) {
                        case sf::Mouse::Left:
                            isDragging = false;
                            break;
                        case sf::Mouse::Right:
                            break;
                        default:
                            break;
                    }
                    break;

                case sf::Event::MouseMoved:
                    if(isDragging) {                  
                        sf::Vector2f diff { event.mouseMove.x - mousePos.x, 
                                            event.mouseMove.y - mousePos.y };
                        originPos += diff;
                        
                        if(diff.x == 0.0f && diff.y == 0.0f)
                            draggingSpeed = { 0.0f, 0.0f };
                        else
                            draggingSpeed += diff;
                    }
                    mousePos = { (float)event.mouseMove.x, (float)event.mouseMove.y };
                    break;
                
                case sf::Event::MouseWheelScrolled:
                    switch(event.mouseWheelScroll.wheel) {
                        case sf::Mouse::VerticalWheel: {
                            float newPixelsPerUnit = pixelsPerUnit - event.mouseWheelScroll.delta * zoomingSpeed;
                            newPixelsPerUnit = std::clamp(newPixelsPerUnit, minPixelsPerUnit, maxPixelsPerUnit);
                            float zoomingFactor = newPixelsPerUnit / pixelsPerUnit;
                            originPos *= zoomingFactor;
                            originPos += mousePos * (1.0f - zoomingFactor);
                            pixelsPerUnit = newPixelsPerUnit;
                            break;
                        }
                        case sf::Mouse::HorizontalWheel:
                            break;
                    }
                    break;

                default:
                    break;
            }
        }

        if (clock.getElapsedTime().asSeconds() >= timeBetweenFrames) {
            window.clear(backgroundColor);
            float elapsedTime = clock.restart().asSeconds();

            // handle dragging speed
            draggingSpeed -= elapsedTime * draggingSpeedDecayTimeMultiplicator * draggingSpeed / 2.0f;
            if(!isDragging) {
                originPos += draggingSpeedMultiplicator * draggingSpeed * elapsedTime;
            }

            // draw grid
            auto getGridVertices = [=](float gridPixelInterval, sf::Color gridColor) {
                std::vector<sf::Vertex> gridVertices;
                for(float x = fmod(originPos.x, gridPixelInterval); x < windowSize.x; x += gridPixelInterval) {
                    gridVertices.push_back(sf::Vertex({ x, 0 }, gridColor));
                    gridVertices.push_back(sf::Vertex({ x, windowSize.y }, gridColor));
                }
                for(float y = fmod(originPos.y, gridPixelInterval); y < windowSize.y; y += gridPixelInterval) {
                    gridVertices.push_back(sf::Vertex({ 0, y }, gridColor));
                    gridVertices.push_back(sf::Vertex({ windowSize.x, y }, gridColor));
                }
                return gridVertices;
            };

            auto subGridVertices = getGridVertices(subGridUnitInterval * pixelsPerUnit, subGridColor);
            auto gridVertices = getGridVertices(gridUnitInterval * pixelsPerUnit, gridColor);

            window.draw(&subGridVertices[0], subGridVertices.size(), sf::Lines);
            window.draw(&gridVertices[0], gridVertices.size(), sf::Lines);

            // draw axis
            sf::Vertex axisLines[] = {
                sf::Vertex({ originPos.x, 0 }, axisColor),
                sf::Vertex({ originPos.x, windowSize.y }, axisColor),
                sf::Vertex({ 0, originPos.y }, axisColor),
                sf::Vertex({ windowSize.x, originPos.y }, axisColor)
            };
            window.draw(axisLines, 4, sf::Lines);

            // draw a function
            auto f = [](float x) {
                return cos(2.0f * x) + cos(3.0f * x);
            };

            std::vector<sf::Vertex> vertices;

            for(float pixelX = 0.0f; pixelX <= windowSize.x; pixelX += 1.0f) {
                float unitX = (pixelX - originPos.x) / pixelsPerUnit;
                float unitY = f(unitX);
                float pixelY = unitY * pixelsPerUnit + originPos.y;
                
                float dUnitX = 0.01f;
                float tangentAngle = atan2(f(unitX - dUnitX) - f(unitX + dUnitX), - 2 * dUnitX) + 3.14159f / 2.0f;

                float thickness = 2.0f;
                vertices.push_back(sf::Vertex({ pixelX + thickness * cos(tangentAngle),
                                                pixelY + thickness * sin(tangentAngle) },
                                              sf::Color(255, 0, 0, 160)));
                vertices.push_back(sf::Vertex({ pixelX + thickness * cos(tangentAngle + 3.14159f),
                                                pixelY + thickness * sin(tangentAngle + 3.14159f) },
                                              sf::Color(255, 0, 0, 160)));
            }

            window.draw(&vertices[0], vertices.size(), sf::TrianglesStrip); 

            // draw axis tick labels 
            auto getText = [&font](int label) {
                sf::Text text(std::to_string(label), font);
                text.setFillColor(sf::Color::Black);
                text.setCharacterSize(24);
                return text;
            };
            float gridPixelInterval = gridUnitInterval * pixelsPerUnit;

            for(float x = fmod(originPos.x, gridPixelInterval); x < windowSize.x; x += gridPixelInterval) {
                int label = round((x - originPos.x) / gridPixelInterval);
                sf::Text labelText = getText(label);
                sf::FloatRect boundingBox = labelText.getLocalBounds();
                sf::Vector2f labelPosition(x - boundingBox.left - boundingBox.width / 2.0f, originPos.y);
                if(label == 0)
                    labelPosition.x = originPos.x - boundingBox.left - boundingBox.top - boundingBox.width;
                labelText.setPosition(labelPosition);
                window.draw(labelText);
            }
            for(float y = fmod(originPos.y, gridPixelInterval); y < windowSize.x; y += gridPixelInterval) {
                int label = round((originPos.y - y) / gridPixelInterval);
                sf::Text labelText = getText(label);
                sf::FloatRect boundingBox = labelText.getLocalBounds();
                sf::Vector2f labelPosition(originPos.x - boundingBox.left - boundingBox.top - boundingBox.width,
                                           y - boundingBox.top - boundingBox.height / 2.0f);
                if(label == 0) labelPosition.y = originPos.y;
                labelText.setPosition(labelPosition);
                window.draw(labelText);
            }

            window.display();
        }
    }

    return 0;
}
