#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <chrono>
#include <algorithm>

using namespace std;
#define _USE_MATH_DEFINES

void rotate(array<double, 3>& point, array<double, 9>& matrix)
{
    point = {point[0] * matrix[0] + point[1] * matrix[1] + point[2] * matrix[2], 
                point[0] * matrix[3] + point[1] * matrix[4] + point[2] * matrix[5],
                point[0] * matrix[6] + point[1] * matrix[7] + point[2] * matrix[8]};    
}

class point
{
    public:
        double x, y, z;
        array<double, 3> origin_vector;
        double brightness = 1;


        point(array<double, 3> pos, array<double, 3> center)
        {
            
            x = pos[0]; y = pos[1]; z = pos[2];
            origin_vector = {x - center[0], y - center[1], z - center[2]};
            double length = sqrt(origin_vector[0] * origin_vector[0] + origin_vector[1] * origin_vector[1] + origin_vector[2] * origin_vector[2]);
            origin_vector = {origin_vector[0] / length, origin_vector[1] /length, origin_vector[2] / length};
        }

        void light(array<double, 3>& mouse_vector)
        {
            brightness = -mouse_vector[0] * origin_vector[0] - mouse_vector[1] * origin_vector[1] + mouse_vector[2] * origin_vector[2];
        }


        void rotate_(array<double, 9>& matrix, const array<double, 3> rotate_point)
        {   
            
            array<double, 3> p = {x - rotate_point[0], y - rotate_point[1], z - rotate_point[2]};
            rotate(p, matrix);x=p[0] + rotate_point[0];y=p[1] + rotate_point[1];z=p[2] + rotate_point[2];
            rotate(origin_vector, matrix);
        }

        sf::Vertex set_vertex(array<double, 2> coord_center)
        {   
            array<double, 2> coord = z_buffer(500);
            sf::Vertex vertex_point;
            vertex_point.position = sf::Vector2f(coord[0] + coord_center[0], coord[1] + coord_center[1]);
            vertex_point.color = sf::Color(max(0.0, 255*brightness), max(0.0, 255*brightness), max(0.0, 255*brightness), 255);
            return vertex_point;
        }


    private:

        array<double, 2> z_buffer(double K_constant)
        {
            return {x * K_constant / z, y * K_constant / z};
        }

};

class quad
{
    public:
        vector<point> points;
        vector<sf::Vertex> vertex_quad;

        quad(vector<point> pos)
        {
            points = pos;
        }

        void light(array<double, 3>& mouse_vector)
        {
            for(point& p : points)p.light(mouse_vector);
        }

        void rotate_(array<double, 9>& matrix, const array<double, 3>& rotate_point)
        {   
            for(point& p : points)
            {
                p.rotate_(matrix, rotate_point);
            }
        }

        void set_vertex(array<double, 2> coord_center)
        {   
            vertex_quad.clear();
            for(point p : points)vertex_quad.push_back(p.set_vertex(coord_center));
        }
};


void define_sphere(vector<quad>& quads, array<array<double, 9>, 3>& rotation_matrices, double radius, double density, double z_dist)
{

    //density in degrees
    array<double, 3> current_point = {0, radius, 0};
    vector<point> previous_section;
    vector<point> current_section;
    for(int i=0; i <= 180/(density); i++)
    {   
        current_section.clear();
        array<double, 3> new_point = current_point;
        for(int j=0; j <= (360/density) + 1; j++)
        {   
            point p(new_point, {0, 0, 0});
            
            current_section.push_back(p);
            rotate(new_point, rotation_matrices[1]);
        }

        if(previous_section.size() != 0)
        {
            for(int j=0; j < previous_section.size() - 1; j++)
            {   
                quad quad_create({previous_section[j], previous_section[j+1], current_section[j+1], current_section[j]});
                quads.push_back(quad_create);
            }
        }
        previous_section = current_section;
        
        rotate(current_point, rotation_matrices[2]);
    }

    for(quad& q : quads)
    {
        for(point& p : q.points)
        {
            p.z += z_dist;
        }
    }

}

void define_donut(vector<quad>& quads, array<array<double, 9>, 3>& rotation_matrices, const array<double, 2>& radius, double density, double z_dist)
{

    array<double, 3> basic_circle = {radius[0], 0, z_dist};
    vector<array<double, 3>> shape;

    array<double, 3> new_point = {radius[1], 0, 0};
    for(int i=0; i < (360/density + 1); i++)
    {
        rotate(new_point, rotation_matrices[1]);
        array<double, 3> tmp = new_point;
        tmp[0] += basic_circle[0];tmp[2] += basic_circle[2];
        shape.push_back(tmp);
    }

    
    for(int i=0; i < (360/density + 1); i++)
    {   
        array<double, 3> old_basic_circle = basic_circle;
        rotate(basic_circle, rotation_matrices[2]);
        vector<array<double, 3>> next_shape = shape;
        for(array<double, 3>& p : next_shape)
        {
            rotate(p, rotation_matrices[2]);
        }
        
        for(int j=0; j < shape.size() - 1; j++)
        {   
            vector<point> q;
            for(int k=0; k < 2; k++)
            {
                point one(shape[j + k], old_basic_circle);
                point two(next_shape[j + k], basic_circle);

                if(k){q.push_back(one);q.push_back(two);}
                else {q.push_back(two);q.push_back(one);}

            }

            quad quad_create(q);
            quads.push_back(quad_create);
        }
        shape = next_shape;
    }

}




int main()
{   
    
    sf::Clock clock;
    sf::RenderWindow window(sf::VideoMode(1000, 1000), "Donut maker", sf::Style::Close);
    double degrees = 5;
    double radians = degrees *  M_PI / 180;
    double sine = sin(radians);double cosine = cos(radians);
    array<array<double, 9>, 3> rotation_matrices = 
                {{
                    //x rotation
                    {1, 0, 0, 
                    0, cosine, -sine,
                    0, sine, cosine},

                    //y rotation
                    {cosine, 0, sine,
                        0, 1, 0,
                    -sine, 0, cosine},

                    //z rotation
                    {cosine, -sine, 0,
                    sine, cosine, 0,
                         0, 0, 1}
                }};
    
    
    vector<quad> quads;sf::VertexArray vertices(sf::Quads);
    array<double, 2> coord_center = {window.getSize().x / 2.0, window.getSize().y / 2.0};
    double z_distance = -700;
                                            //radius     /z_distance
    //define_sphere(quads, rotation_matrices, 200, degrees, z_distance);
    define_donut(quads, rotation_matrices, {200, 50}, degrees, z_distance);
    sort(quads.begin(), quads.end(), [] (const quad& q1, const quad& q2){return q1.points[0].z < q2.points[0].z;});

    int frame_count = 0;

    while(window.isOpen())
    {
        frame_count++;
        window.clear(sf::Color::Black);
        vertices.clear();
        
        array<double, 3> mouse_vector = {(double)(sf::Mouse::getPosition(window).x - ((double)window.getSize().x / 2)), 
                                        (double)(sf::Mouse::getPosition(window).y - ((double)window.getSize().y / 2)), 
                                        500 - 0.0};
        double length = sqrt(mouse_vector[0] * mouse_vector[0] + mouse_vector[1] * mouse_vector[1] + mouse_vector[2] * mouse_vector[2]);
        mouse_vector = {mouse_vector[0] / length, mouse_vector[1] /length, mouse_vector[2] /length};

        if(frame_count%10==0)
        {
            for(quad& q : quads)
            {
                q.rotate_(rotation_matrices[2], {0, 0, z_distance});
                q.rotate_(rotation_matrices[0], {0, 0, z_distance});
            }
            sort(quads.begin(), quads.end(), [] (const quad& q1, const quad& q2){return q1.points[0].z < q2.points[0].z;});
        }

        for(quad& q : quads)
        {   
            q.light(mouse_vector);
            q.set_vertex(coord_center);
            for(sf::Vertex x : q.vertex_quad)vertices.append(x);
        }
        window.draw(vertices);


        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                default:break;
            }
           
        }
        window.display();        
    }
    
    return 0;
}


