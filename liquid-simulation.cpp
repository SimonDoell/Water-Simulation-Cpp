#include <SFML/Graphics.hpp>
#include <list>
#include <vector>
#include <iostream>
#include <random>

const int WIDTH = 1920;
const int HEIGHT = 1010;
const int PARTICLES = 1500;
const float DRAG = 0.99f;
const float BOUNCE_ENERGY_ABSORB = 0.6f;
const int GRID_SIZE = 50;
const int GRID_RADIUS = 5;
//--> 5 for great Quality; 3 for faster, 3 will cause Artifacts of the grid optimization
float forceCutoffRange = 125.0;
float wallForceCutoff = 100.0f;
float highestForcetemp = 0.0f;


sf::Vector2f GRAVITY(0, 0.15);
int MAX_FRAMES = 60;


float forceEquation(float distance) {
    float maxRange = 3;
    float height = 0.1;
    float forceCutoffRange = maxRange;

    if (distance >= forceCutoffRange)
        return 0.f;

    float baseForce = (1.f / (distance + height)) - (1.f / (maxRange + height));

    float fade = 0.5f * (1 + std::cos((distance / forceCutoffRange) * M_PI));

    return baseForce * fade;
}

float wallForceEquation(float distance) {
    float force = float(pow((abs(distance)+1), -2));
    //std::cout << "Force: " << force << "\n";
    //if (force > highestForcetemp) {
    //    highestForcetemp = force;
    //    std::cout << "New high: " << highestForcetemp << " ,with distance: " << distance << "\n";
    //}
    if (force > 0.1f) {
        return float(0.1f);
    } else {return force;}
}




class Particle {
    float mass;
    float radius;
    sf::Vector2f velocity;
    sf::Vector2f pos;
    sf::CircleShape obj;

    public:
        Particle(float _mass, float _radius, sf::Vector2f _pos, sf::Vector2f _velocity) : mass(_mass), radius(_radius), pos(_pos), velocity(_velocity) {
            obj.setRadius(radius);
            obj.setFillColor(sf::Color::White);
            obj.setPosition(pos);
        }
    
        void physicsUpdate(sf::Vector2f ownGridVector, std::vector<std::vector<std::vector<int>>>& gridMap, std::vector<Particle>& particles, int index, std::vector<sf::Vector2f>& newVelocity) {
            

            std::vector<sf::Vector2i> squareVectorMapCoords = {};
            int iterationCount = GRID_RADIUS*2+1;

            //std::cout << "Starting Loop..." << "\n";

            for (int i = 0; i < iterationCount; ++i) {
                for (int ii = 0; ii < iterationCount; ++ii) {
                    int vectorX = ownGridVector.x + (i - GRID_RADIUS);
                    int vectorY = ownGridVector.y + (ii - GRID_RADIUS);
                    if (vectorX >= 0 && vectorX <= gridMap.size()-1 && vectorY >= 0 && vectorY <= gridMap[0].size()-1) {
                        //std::cout << "Emplaced: (" << vectorX << " | " << vectorY << "), with gridMap Size: (" << gridMap.size() << " | " << gridMap[0].size() << ")\n";
                        squareVectorMapCoords.emplace_back(sf::Vector2i(vectorX, vectorY));
                    }
                    
                }
            }

            //std::cout << "Staring Loop 2...\n";
            for (int i = 0; i < squareVectorMapCoords.size(); ++i) {
                //std::cout << "Done I: " << i << "\n";



                for (int ii = 0; ii < gridMap[squareVectorMapCoords[i].x][squareVectorMapCoords[i].y].size(); ++ii) {

                    //std::cout << "Doing II: " << ii << ", with Coords: (" << squareVectorMapCoords[i].x << " | " << squareVectorMapCoords[i].y << ")" << "...\n";
                    int index = gridMap[squareVectorMapCoords[i].x][squareVectorMapCoords[i].y][ii];
                    if (index < 0 || index >= particles.size()) {
                        continue;
                    }


                    //Code for looping over every particle and applying forces
                    sf::Vector2f forceVector = particles[index].getPos() - pos;
                    float forceVectorLength = sqrt(forceVector.x*forceVector.x + forceVector.y*forceVector.y);
                    if (forceVectorLength > 1e-6f) {
                        float forceEquationMuliplier = forceEquation(forceVectorLength/100.0f);
                        forceVector /= forceVectorLength;
                        //--> Normalisierung
                        forceVector *= forceEquationMuliplier/100.0f;
                        //Muliplizieren der asugerechneten Kraft
                        velocity -= forceVector;
                        particles[index].translateVelocity(forceVector);


                        if (WIDTH - pos.x <= wallForceCutoff) {
                            velocity.x -= wallForceEquation(WIDTH-pos.x);
                        } else if (pos.x <= wallForceCutoff) {
                            velocity.x += wallForceEquation(pos.x);
                        }

                        if (HEIGHT - pos.y <= wallForceCutoff) {
                            velocity.y -= wallForceEquation(HEIGHT-pos.y);
                        } else if (pos.y <= wallForceCutoff) {
                            velocity.y += wallForceEquation(pos.y);
                        }
                       
                        
                    }


                    

                   
                    
                    //particles[index].setFillColor(sf::Color::Red);
                }
            }
            //obj.setFillColor(sf::Color::Green);

            velocity *= DRAG;
            //velocity += GRAVITY;
            newVelocity.emplace_back(velocity);
        }


        void setFillColor(sf::Color color) {
            obj.setFillColor(color);
        }
        sf::Vector2f getVelocity() {
            return velocity;
        }
        void translateVelocity(sf::Vector2f vel) {
            velocity += vel;
        }
        sf::Vector2f getPos() {
            return pos;
        }
        void move() {
            pos += velocity;
            obj.setPosition(pos);
        }
        void draw(sf::RenderWindow& window) {
            window.draw(obj);
        }
        void checkBounds() {
            if (pos.x <= 0) {
                pos.x = 0;
                velocity.x *= -BOUNCE_ENERGY_ABSORB;
            } else if (pos.x + 2*radius >= WIDTH) {
                pos.x = WIDTH - 2*radius;
                velocity.x *= -BOUNCE_ENERGY_ABSORB;
            }

            if (pos.y <= 0) {
                pos.y = 0;
                velocity.y *= -BOUNCE_ENERGY_ABSORB;
            } else if (pos.y + 2*radius >= HEIGHT) {
                pos.y = HEIGHT - 2*radius;
                velocity.y *= -BOUNCE_ENERGY_ABSORB;
            }
        }
        void colorGridDebug(sf::Vector2i gridVector) {
            if (gridVector.x % 2 == 0) {
                obj.setFillColor(sf::Color::Red);
            } else if (gridVector.y % 2 == 0) {
                obj.setFillColor(sf::Color::Green);
            } else {
                obj.setFillColor(sf::Color::White);
            }
        }
        void colorVelocity(
            const std::vector<sf::Vector2f>& oldVelocity,
            const std::vector<sf::Vector2f>& newVelocity
        ) {
            const float minSpeed = 0.0f;
            const float maxSpeed = 3.0f; // adjust this to your max expected speed

            for (size_t i = 0; i < newVelocity.size(); ++i) {
                sf::Vector2f v = newVelocity[i];
                float speed = std::sqrt(v.x * v.x + v.y * v.y);
                float normalized = std::min(speed / maxSpeed, 1.0f);
                int blue = static_cast<int>(normalized * 155 + 100);
                blue = std::min(255, std::max(0, blue));

                obj.setFillColor(sf::Color(0, 0, blue, 255));
            }
        }


        sf::Vector2f getGridVector() {
            float grid_x = int(pos.x / GRID_SIZE);
            float grid_y = int(pos.y / GRID_SIZE);
            //std::cout << "GridVector: (" << grid_x << " | " << grid_y << ")\n";
            return sf::Vector2f(grid_x, grid_y);
        }
};


int main() {
    //Main Windows settings
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "WindowTitle", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(MAX_FRAMES);
    sf::Event ev;
    sf::ContextSettings settings;
    //settings.antialiasingLevel = 2;



    //Space for creating lists:
    std::vector<Particle> particles;
    std::vector<std::vector<std::vector<int>>> grid;
    std::vector<sf::Vector2f> oldVelocity;
    std::vector<sf::Vector2f> newVelocity;
   


    //Setting variables for Object:
    for (int i = 0; i <= PARTICLES; i++) {
        particles.emplace_back(Particle(1.0f, 3.0f, sf::Vector2f(rand() % WIDTH, rand() % HEIGHT), sf::Vector2f(float((rand()%101)-50)/-50, float((rand()%101)-50)/-50)));
    }


    //particles[4550].setVelocity(sf::Vector2f(1, 0.5));
    
    




    //Main Loop/Game Loop
    while (window.isOpen()) {
        //Window Management:
        while (window.pollEvent(ev))
        {
           switch(ev.type) 
           {
            case sf::Event::Closed:
                window.close();
                break;
           }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window.close();
        


        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {

            sf::Vector2i mousePos = sf::Mouse::getPosition();
            float dist = 100.0f;

            for (int i = 0; i < particles.size(); ++i) {
                sf::Vector2f particlePos = particles[i].getPos();
                float dx = mousePos.x - particlePos.x;
                float dy = mousePos.y - particlePos.y;
                float distance = sqrt(dx*dx + dy*dy);
                //if (distance < dist) {
                //    particles[i].translateVelocity(sf::Vector2f(1, 0));
                //}
                if (abs(dx) < dist) {
                    particles[i].translateVelocity(sf::Vector2f(1, 0));
                }
            };
        };
        



        grid = {};
        int gridWidth = int(WIDTH / GRID_SIZE) + 1 + GRID_RADIUS * 2;
        int gridHeight = int(HEIGHT / GRID_SIZE) + 1 + GRID_RADIUS * 2;
        grid.resize(gridWidth);
        for (int i = 0; i < gridWidth; i++) {
            grid[i].resize(gridHeight);
        }

        oldVelocity = {};
        for (int i = 0; i < particles.size(); ++i) {
            //particles[i].setFillColor(sf::Color::Blue);
            sf::Vector2f gridVector = particles[i].getGridVector();
            grid[gridVector.x][gridVector.y].emplace_back(i);
            oldVelocity.emplace_back(particles[i].getVelocity());
            //particles[i].colorGridDebug(sf::Vector2i(int(gridVector.x), int(gridVector.y)));
            if (i == particles.size()-1) {
                for (int ii = 0; ii < particles.size(); ++ii) {
                    particles[ii].physicsUpdate(particles[ii].getGridVector(), grid, particles, ii, newVelocity);
                    particles[ii].colorVelocity(oldVelocity, newVelocity);
                    newVelocity = {};
                }
            }
        }

        for (int i = 0; i < particles.size(); ++i) {
            particles[i].move();
            particles[i].checkBounds();
        }




        //Rendering:
        window.clear();
        //std::cout << ".render()" << "\n";
        //std::cout << "Done" << "\n";
        for (int i = 0; i < particles.size(); ++i) {
            particles[i].draw(window);
        }
        window.display();

    };

    return 0;
};
