#ifndef SPHEREHEADER
#define SPHEREHEADER
#include <iostream>
#include <cmath>
#include "Object.cpp"
#include "Vector.cpp"
#include "Point.cpp"
#include "Ray.cpp"
#include "AABB.cpp"

class sphere : public object{
    private:
        double ta;
        ray* ra;

        //iluminação
        vetor kd;  // Difuso
        vetor ks;  // Specular
        vetor ke;  // Emissivo
        vetor ka;  // Ambiente
        double ns; // Brilho
        double ni; // Índice de refração
        double d;  // Opacidade

    public:
        point center;
        double radius;

        sphere(point center,
               double radius,
               vetor color,
               vetor kd,
               vetor ks,
               vetor ke,
               vetor ka,
               double ns,
               double ni,
               double d) : 

        object(color),
        radius(radius),
        center(center),
        kd(kd),
        ks(ks),
        ke(ke),
        ka(ka),
        ns(ns),
        ni(ni),
        d(d) {}

        point getPonto() override { return this->center; }

        vetor getKd() override {return kd;}
        vetor getKs() override {return ks;}
        vetor getKe() override {return ke;}
        vetor getKa() override {return ka;}
        double getNi() override {return ni;}
        double getD() override {return d;}
        double getShininess() override { return this->ns; }

        double intersect(ray &r) override {
            double t;

            //std::clog << "entrando na função de intersessão da ESFERA" << std::endl;

            //std::clog << "esfera: (" << center.getX() << "," <<
            //center.getY() << ", "<< center.getZ() << ") " << std::endl;

            //std::clog << "ray origin: (" << r.origin.getX() << "," <<
            //r.origin.getY() << ", "<< r.origin.getZ() << ") " << std::endl;

            vetor oc = (r.origin) - (this->getPonto());
            double a = r.direction.produto_escalar(r.direction);
            double b = 2.0 * oc.produto_escalar(r.direction);
            double c = oc.produto_escalar(oc) - (this->radius * this->radius);
            double delta = (b * b) - (4 * a * c);

            if (delta >= 0)
            {
                t = (-b - sqrt(delta)) / 2 * a;

                if (t < 0){
                    t = (-b + sqrt(delta)) / 2 * a;
                }
            }
            else{
                t = -1;
            }
            this->ra = &r;
            this->ta = t;

            //std::clog << "t: " << t << std::endl;
            
            return t;
            
        }


        virtual AABB getAABB() override {
            // Assuming BoundingBox has min and max points
            point min_point(center.getX() - radius, center.getY() - radius, center.getZ() - radius);
            point max_point(center.getX() + radius, center.getY() + radius, center.getZ() + radius);
            return AABB(min_point, max_point);
        }
  
        virtual vetor getNormal() override {
            point ponto_intersecao = ra->f(this->ta);
            return (ponto_intersecao - this->center).normalizar();
        };

        void translacao(double dx, double dy, double dz) override {
            this->center = this->center + point(dx, dy, dz);
        }

        void rotacao(double angle, char eixo, point centroide) override {};
        void cisalhamento(double shXY, double shXZ, double shYX, double shYZ, double shZX, double shZY) override {};
};
#endif
