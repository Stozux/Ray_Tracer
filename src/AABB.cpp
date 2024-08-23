#ifndef AABBHEADER
#define AABBHEADER
#include <vector>
#include "Point.cpp"
#include "Ray.cpp"
#include "Object.cpp"

class AABB {
public:
    point min;
    point max;

    AABB( point& min_point,  point& max_point)
        : min(min_point), max(max_point) {}

    bool intersects( ray& r)  {
        // Initialize t_min and t_max to the ray's possible intersection intervals
        double t_min = (min.getX() - r.getOrigin().getX()) / r.getDirection().getX();
        double t_max = (max.getX() - r.getOrigin().getX()) / r.getDirection().getX();

        // Ensure t_min is the minimum and t_max is the maximum for the X axis
        if (t_min > t_max) std::swap(t_min, t_max);

        // Calculate the intersection intervals for the Y axis
        double ty_min = (min.getY() - r.getOrigin().getY()) / r.getDirection().getY();
        double ty_max = (max.getY() - r.getOrigin().getY()) / r.getDirection().getY();

        // Ensure ty_min is the minimum and ty_max is the maximum for the Y axis
        if (ty_min > ty_max) std::swap(ty_min, ty_max);

        // If intervals don't overlap, there is no intersection
        if ((t_min > ty_max) || (ty_min > t_max)) {
            return false;
        }

        // Update t_min and t_max to include the Y axis intersection interval
        if (ty_min > t_min) t_min = ty_min;
        if (ty_max < t_max) t_max = ty_max;

        // Calculate the intersection intervals for the Z axis
        double tz_min = (min.getZ() - r.getOrigin().getZ()) / r.getDirection().getZ();
        double tz_max = (max.getZ() - r.getOrigin().getZ()) / r.getDirection().getZ();

        // Ensure tz_min is the minimum and tz_max is the maximum for the Z axis
        if (tz_min > tz_max) std::swap(tz_min, tz_max);

        // If intervals don't overlap, there is no intersection
        if ((t_min > tz_max) || (tz_min > t_max)) {
            return false;
        }

        // Update t_min and t_max to include the Z axis intersection interval
        if (tz_min > t_min) t_min = tz_min;
        if (tz_max < t_max) t_max = tz_max;

        // If t_min is less than or equal to t_max, the ray intersects the AABB
        return t_max >= std::max(t_min, 0.0);
    }


    // AABB-AABB intersection test (if needed)
    bool intersects(AABB& other){
        // Check for overlap in all three axes
        return (min.getX() <= other.max.getX() && max.getX() >= other.min.getX()) &&
               (min.getY() <= other.max.getY() && max.getY() >= other.min.getY()) &&
               (min.getZ() <= other.max.getZ() && max.getZ() >= other.min.getZ());
    }

};

#endif
