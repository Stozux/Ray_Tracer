#include <algorithm>
#include <complex>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "src/Camera.cpp"
//#include "src/CameraOriginal.cpp"
#include "src/Plane.cpp"
#include "src/Vector.cpp"
#include "src/Sphere.cpp"
#include "src/AABB.cpp"
#include "src/Octrees.cpp"
#include "src/Object.cpp"

using namespace std;

AABB calculateSceneAABB( std::vector<object*>& objects) {
    if (objects.empty()) {
        // Define a default bounding box if no objects are present
        point mn,mx;
        mn = point(0.1, 0.1, 0.1);
        mx = point(1000,1000,1000);
        return AABB(mn, mx);
    }

    // Initialize min and max with the first object's bounding box
    AABB scene_box = objects[0]->getAABB();

    /*
    std::clog << "scene_box min: (" << scene_box.min.getX() << "," <<
    scene_box.min.getY() << ", " << scene_box.min.getZ() << ") " << std::endl;

    std::clog << "scene_box max: (" << scene_box.max.getX() << "," <<
    scene_box.max.getY() << ", "<< scene_box.max.getZ() << ") " << std::endl;
    //*/

    for ( auto& obj : objects) {
        AABB obj_box = obj->getAABB();

        // tirando isso pra não dar erro de intersessão
        /*
        scene_box.min.setX(std::min(scene_box.min.getX(), obj_box.min.getX()));
        scene_box.min.setX(std::min(scene_box.min.getY(), obj_box.min.getY()));
        scene_box.min.setX(std::min(scene_box.min.getZ(), obj_box.min.getZ()));
        */

        scene_box.min = point(0.1, 0.1, 0.1);


        scene_box.max.setX(std::max(scene_box.max.getX(), obj_box.max.getX()));
        scene_box.max.setX(std::max(scene_box.max.getY(), obj_box.max.getY()));
        scene_box.max.setX(std::max(scene_box.max.getZ(), obj_box.max.getZ()));
    }

    return scene_box;
}

int main() {
    ios::sync_with_stdio(false); cin.tie(NULL);

    point pos_cam(0, 0, -1);  // Move the camera closer (5 units away from the origin)
    point target_cam(0, 0, 0); // Looking towards the origin
    vetor up_cam(0, 1, 0);     // Up direction along the Y-axis

    camera cam(1600, pos_cam, target_cam, up_cam, 16.0/9.0, 1.0);

    vector<object*> spheres;

    // Sphere parameters
    double radius = 0.05;  // Small radius to ensure no intersection
    vetor kd = vetor(0.5, 0, 0);
    vetor ks = vetor(0, 0, 0);
    vetor ke = vetor(0.000000, 0.000000, 0.000000);
    vetor ka = vetor(1, 1, 1);
    double ns = 10.0;
    double ni = 1.0;  // Refractive index
    double d = 1.0;   // Fully opaque

    // Calculate spacing
    double x_spacing = 3.555 / 5.0; // Horizontal spacing based on viewport width
    double y_spacing = 2.0 / 4.0;   // Vertical spacing based on viewport height
    
    double x = 0;  // Centered horizontally
    double y = 0;    // Centered vertically

    point center(x, y, 1.5);  // Move spheres closer to the camera along the Z-axis

    sphere* esfera = new sphere(center, radius, vetor(1,1,1), kd, ks, ke, ka, ns, ni, d);
    //spheres.push_back(esfera);

    for (int i = 0; i < 10; ++i) {  // Rows
        for (int j = 0; j < 10; ++j) {  // Columns
            double x = -3.2 + j * x_spacing;  // Centered horizontally
            double y = -2.2 + i * y_spacing;    // Centered vertically

            point center(x, y, 1.5);  // Move spheres closer to the camera along the Z-axis

            sphere* esfera = new sphere(center, radius, vetor(0,0,1), kd, ks, ke, ka, ns, ni, d);
            spheres.push_back(esfera);
        }
    }
    //

    //- -------------------
    // Calculate the scene's bounding box
    //AABB sceneBounds = calculateSceneAABB(spheres);
    point a(-4,-4,0);
    point b(4,4,8);
    AABB sceneBounds(a,b);

    // Initialize the octree with the scene's bounding box
    Octree octree(sceneBounds);

    // Insert all spheres into the octree
    for (auto obj : spheres) {
        octree.insert(obj);
        //std::clog << "chamou octree.insert" << std::endl;
    }

    std::clog << "main - octree objs: " << octree.objects.size() << std::endl;

    // Render the scene using the octree
    vetor ambient_light(1, 1, 1);
    std::vector<light> lights; // Define lights as needed
    cam.render(octree, lights, ambient_light);
    //cam.render(spheres, lights, ambient_light);

    return 0;
}
