#ifndef OCTREE
#define OCTREE
/*
https://stackoverflow.com/questions/10228690/ray-octree-intersection-algorithms

-- blog simples
https://daeken.svbtle.com/a-stupidly-simple-fast-octree-traversal-for-ray-intersection

-- github
https://github.com/Aransentin/galena/tree/master/src

-- reverter render pra coisa normal? ou usar entrega 5 msm

--appel
*/
#include <vector>
#include "AABB.cpp"

class Octree {
public:
    AABB bounds;               // Bounding box of this node
    std::vector<object*> objects;     // Objects contained in this node
    Octree* children[8];              // Child nodes
    int maxObjects;                   // Maximum objects before subdivision
    int maxDepth;                     // Maximum depth of the octree
    int currentDepth;                 // Current depth of this node
    // Constructor

    Octree(const AABB& b, int depth = 0, int max_depth = 8, int max_objs = 4)
        : bounds(b), maxDepth(max_depth), maxObjects(max_objs), currentDepth(depth) {
        for (int i = 0; i < 8; ++i) children[i] = nullptr;
    }

    // Destructor to clean up child nodes
    ~Octree() {
        for (int i = 0; i < 8; ++i) {
            if (children[i]) {
                delete children[i];
                children[i] = nullptr;
            }
        }
    }

    // Method to insert an object into the octree
    void insert(object* obj) {
        // If this node has children, delegate the insertion to the appropriate child
        if (children[0] != nullptr) {
            AABB obj_box = obj->getAABB();
            int index = getChildIndex(obj_box);
            if (index != -1) {
                children[index]->insert(obj);
                return;
            }
        }

        // Add the object to this node
        objects.push_back(obj);
        //std::clog << "objeto de fato adicionado em nó" << std::endl;

        // Subdivide if necessary
        if (objects.size() > maxObjects && currentDepth < maxDepth) {
            if (children[0] == nullptr) {
                subdivide();
            }

            // Re-distribute objects to children
            for (auto it = objects.begin(); it != objects.end(); ) {
                AABB obj_box = (*it)->getAABB();
                int index = getChildIndex(obj_box);

                //index = 7;

                //std::clog << "idx: "  << index << std::endl;
                
                if (index != -1) {
                    children[index]->insert(*it);
                    //std::clog << "inseriu no filho" << std::endl;
                    it = objects.erase(it);
                } else {
                    ++it;
                }
            }
            //objects.clear();
        }
    }

    // Method to subdivide this node into 8 children
    void subdivide() {
        // Calculate the center of the current bounding box
        double midX = (bounds.min.getX() + bounds.max.getX()) / 2.0;
        double midY = (bounds.min.getY() + bounds.max.getY()) / 2.0;
        double midZ = (bounds.min.getZ() + bounds.max.getZ()) / 2.0;

        // Define the 8 children bounding boxes
        for (int i = 0; i < 8; ++i) {
            double newMinX = (i & 1) ? midX : bounds.min.getX();
            double newMaxX = (i & 1) ? bounds.max.getX() : midX;
            double newMinY = (i & 2) ? midY : bounds.min.getY();
            double newMaxY = (i & 2) ? bounds.max.getY() : midY;
            double newMinZ = (i & 4) ? midZ : bounds.min.getZ();
            double newMaxZ = (i & 4) ? bounds.max.getZ() : midZ;


            point mnpoint = point(newMinX, newMinY, newMinZ);
            point mxpoint = point(newMaxX, newMaxY, newMaxZ);
            AABB childBounds = AABB(mnpoint, mxpoint);
            children[i] = new Octree(childBounds, currentDepth + 1, maxDepth, maxObjects);
        }
    }

    // Helper method to determine which child node an object's bounding box fits into
    // Returns -1 if the object does not fit entirely within any child node
    int getChildIndex( AABB& objBox) {
        // Determine which octant the objectBox belongs to
        // If the object spans multiple octants, return -1

        // Calculate the center of the bounding box
        double midX = (bounds.min.getX() + bounds.max.getX()) / 2.0;
        double midY = (bounds.min.getY() + bounds.max.getY()) / 2.0;
        double midZ = (bounds.min.getZ() + bounds.max.getZ()) / 2.0;

        bool left = objBox.max.getX() < midX;
        bool right = objBox.min.getX() >= midX;
        bool bottom = objBox.max.getY() < midY;
        bool top = objBox.min.getY() >= midY;
        bool back = objBox.max.getZ() < midZ;
        bool front = objBox.min.getZ() >= midZ;

        int index = 0;
        if (right) index |= 1;
        if (top) index |= 2;
        if (front) index |= 4;

        
        // Check if the object fits entirely within one octant
        if ((left && right) || (bottom && top) || (back && front)) {
            return -1; // Spans multiple octants
        }

        return index;
    }

    // Method to find the closest intersection in the octree
    double findClosestIntersection( ray& r, object*& hitObject)  {
        // First, check if the ray intersects this node's bounding box
        //std::clog << "a" << std::endl;

        //if (r.direction.getX() == 0.0263581 && r.direction.getY() == 0.579877 && r.direction.getZ() == -0.814277)
        //std::clog << "existe esse raio" << std:: endl;

        if (!bounds.intersects(r)) {
            return INFINITY;
        }

        //if (r.direction.getX() == 0.0263581 && r.direction.getY() == 0.579877 && r.direction.getZ() == -0.814277)
        //std::clog << "não tá out of bounds" << std:: endl;

        double closestT = INFINITY;

        // Check intersections with objects in this node
        for (const auto& obj : objects) {
            
            //std::clog << "tem um objeto no nó" << std::endl;
            //std::clog << obj std::endl;

            double t = obj->intersect(r);

            if (t > 0.0 && t < closestT) {
                //std::clog << "INTERSESSÃO" << std::endl;
                closestT = t;
                hitObject = obj;
            }
        }

        // Recursively check intersections in child nodes
        for (int i = 0; i < 8; ++i) {
            if (children[i]) {
                object* childHitObject = nullptr;
                double t = children[i]->findClosestIntersection(r, childHitObject);
                if (t > 0.0 && t < closestT) {
                    closestT = t;
                    hitObject = childHitObject;
                }
            }
        }

        //std::clog << "b" << std::endl;

        return closestT;
    }
};








#endif