#ifndef CAMRHEADER
#define CAMRHEADER

#include <iostream>
#include <tuple>
#include <vector>
#include "Object.cpp"
#include "Point.cpp"
#include "Ray.cpp"
#include "Sphere.cpp"
#include "Triangle.cpp"
#include "Light.cpp"
#include "Vector.cpp"

using namespace std;

struct BSP_node {
    BSP_node *atras, *afrente;
    vector<triangle*> node_triangles;
    vetor normal;
    point origem;

    BSP_node() : atras(nullptr), afrente(nullptr) {}
};

class camera 
{
private:
    point position;
    point target;
    vetor up;
    vetor W; // Vetor direção da câmera
    vetor V; // Vetor vertical da câmera
    vetor U; // Vetor horizontal da câmera
    double distance;
    int height;
    int width;
    double aspect_ratio;
    double focal_length;
    double viewport_height;

public:
    point getPosition() const { return position; }
    point getTarget() const { return target; }
    vetor getUp() const { return up; }
    vetor getW() const { return W; }
    vetor getV() const { return V; }
    vetor getU() const { return U; }
    double getDistance() const { return distance; }
    int getHeight() const { return height; }
    int getWidth() const { return width; }
    double getAspectRatio() const { return aspect_ratio; }
    double getFocalLength() const { return focal_length; }
    double getViewportHeight() const { return viewport_height; }

    camera (
        int width,
        point p,
        point t,
        vetor u,
        //double d = 1.0,
        double aspect_ratio = 16.0 / 9.0,
        double focal_length = 1,
        double viewport_height = 2.0
    ) {
        this->position = p;
        this->width = width;
        this->target = t;
        this->up = u.normalizar();
        //this->distance = d;
        this->aspect_ratio = aspect_ratio;
        this->focal_length = focal_length;
        this->viewport_height = viewport_height;

        // Calcula os vetores ortonormais da câmera
        this->W = (target - position).normalizar();
        this->U = up.produto_vetorial(W).normalizar();
        this->V = W.produto_vetorial(U).normalizar();
    }

    camera() {}

    void print() {
        cout << "Camera:" << endl;
        position.print();
        target.print();
        up.print();
        W.print();
        V.print();
        U.print();
    }

    double ray_color(ray& r, object& s) {
        double t = s.intersect(r);
        if (t > 0.0 && t != INFINITY) {
            return t;
        }
        return INFINITY;
    }

    // Função auxiliar para calcular o vetor refletido
    vetor reflect(const vetor& L, const vetor& N) {
        double dotProduct = L.produto_escalar(N);
        vetor temp = 2.0 * dotProduct * N;
        return (temp - L);
    }

    // Função auxiliar para calcular o vetor refratado

    vetor refract(vetor& incident, vetor& normal, double n1, double n2) {
        double coso = incident.produto_escalar(normal);
        double seno = sqrt(1 - (coso * coso));
        double senot = (n2 / n1) * seno;
        double cosot = sqrt(1 - senot * senot);
        double cons = (cosot - ((n1 / n2) * coso));
        vetor t = (incident * (n1 / n2)) - (normal * cons);
        return t;
    }

    vetor phong_shading(ray& r, vector<triangle*>& objetos, const vector<light>& lights, vetor ambient_light, int index)
    {
        vetor final_color(0,0,0);
        if(index<=100)    // pode ser mudado
        {   
            double t = INFINITY;
            double ind = 0;
            for(int k = 0; k < objetos.size(); k++)
            {
                double result = ray_color(r, *objetos[k]);
                if(result > 0.0 and result < t)
                {
                    t = result;
                    ind = k;
                }
            }
            if(t!=INFINITY)
            {   
                index++;
                point intersection = r.f(t);
                vetor normal = objetos[ind]->getNormal().normalizar();
                vetor objeto_color = objetos[ind]->getColor();
                
                final_color =  final_color + (objetos[ind]->getKa().getX() * ambient_light); // componente ambiente
                
                vetor view_dir = r.getDirection().normalizar();
                vetor view_spec = (r.getOrigin() - intersection).normalizar();
                for (const auto& light : lights) {
                    vetor light_dir = (light.getPosition() - intersection).normalizar();
                    
                    vetor reflect_dir = reflect(light_dir, normal);
                    
                    // Componente difusa
                    double diff = std::max(light_dir.produto_escalar(normal), 0.0);
                    final_color = final_color + ((objetos[ind]->getKd().getX() * objeto_color) * diff);                            

                    // Componente especular
                    double spec = pow(std::max(view_spec.produto_escalar(reflect_dir), 0.0), objetos[ind]->getShininess());
                    final_color = final_color + (light.getColor() * spec) * (objetos[ind]->getKs().getX());

                }

                // Componente reflexão
                if(objetos[ind]->getD() != 0) {   
                    vetor dir_reflec_ray = reflect(view_dir*(-1), normal);
                    point new_intersection = intersection + dir_reflec_ray * 0.00001;
                    ray reflec_ray(new_intersection, dir_reflec_ray);
                    vetor reflec_color = phong_shading(reflec_ray,objetos,lights, ambient_light, index);
                    reflec_color = reflec_color*objetos[ind]->getD();
                    final_color = final_color + reflec_color;

                }

                
                // Componente refração
                if (objetos[ind]->getNi() != 0) {
                    double n1 = 1; // Índice de refração do ar
                    double n2 = objetos[ind]->getNi(); // Índice de refração do objeto
                    vetor dir_refrac_ray;
                    /*if(index==1) */
                    /*{*/
                    /*    dir_refrac_ray = refract(view_spec, normal, n1, n2);*/
                    /*    dir_refrac_ray = dir_refrac_ray * -1;*/
                    /*}*/
                    dir_refrac_ray = refract(view_dir, normal, n1, n2);
                    if (dir_refrac_ray.norma() != 0) { // Verifica se não houve reflexão total
                        point new_intersection = intersection + dir_refrac_ray * 0.00001;
                        ray refrac_ray(new_intersection, dir_refrac_ray);
                        vetor refrac_color = phong_shading(refrac_ray,objetos,lights, ambient_light, index);
                        refrac_color = refrac_color * (objetos[ind]->getNi());
                        final_color = final_color + (refrac_color);
                    }
                }
            }
        }
        return final_color;
    }


    bool ray_intersects_node_plane(ray& ray, struct BSP_node* head) {
        vetor plano_orig = vetor(head->origem.getX(), head->origem.getY(), head->origem.getZ());
        vetor raio_orig = vetor(ray.getOrigin().getX(), ray.getOrigin().getY(), ray.getOrigin().getZ());

        double t = (head->normal.produto_escalar(plano_orig) - head->normal.produto_escalar(raio_orig))/ head->normal.produto_escalar(ray.getDirection());

        return (t > 0 && t != INFINITY);

    }


    triangle* search_first_intersection(ray& ray, BSP_node* head, bool& intersectou, triangle* result) {
        /*clog << "buscando intersecao" << endl;*/
        if (head == nullptr) {
            return nullptr; // Caso base: nó nulo
        }

        int c0 = classify_point(ray.origin, head->normal, head->origem);

        // Se não encontrar no nó atual, checa os filhos
        /*result = nullptr;*/

        if (c0 == 1) {
            result = search_first_intersection(ray, head->afrente, intersectou, result);
            if (intersectou) return result;

            // Checa interseção com os triângulos do nó atual
            for (auto tri : head->node_triangles) {
                if (ray_color(ray, *tri) != INFINITY) { // Função que verifica se o raio intersecta o triângulo
                    if (result == nullptr || (tri->getPonto() - ray.origin).norma() < (result->getPonto() - ray.origin).norma()) {
                        intersectou = true;
                        result = tri;
                        return tri;
                    }
                }
            }

            if (!intersectou && ray_intersects_node_plane(ray, head)) {
                result = search_first_intersection(ray, head->atras, intersectou, result);
                if (intersectou) return result;
            }

        } else {
            result = search_first_intersection(ray, head->atras, intersectou, result);
            if (intersectou) return result;

            // Checa interseção com os triângulos do nó atual
            for (auto tri : head->node_triangles) {
                if (ray_color(ray, *tri) != INFINITY) { // Função que verifica se o raio intersecta o triângulo
                    if (result == nullptr || (tri->getPonto() - ray.origin).norma() < (result->getPonto() - ray.origin).norma()) {
                        intersectou = true;
                        result = tri;
                        return tri;
                    }
                }
            }

            if (!intersectou && ray_intersects_node_plane(ray, head)) {
                result = search_first_intersection(ray, head->afrente, intersectou, result);
                if (intersectou) return result;
            }

        }

        return result;
    }


    vetor phong_shading_BSP(ray& r, struct BSP_node* objetos, const vector<light>& lights, vetor ambient_light, int index)
    {
        /*clog << "entrou no phong" << endl;*/
        vetor final_color(0,0,0);
        if(index<=100)    // pode ser mudado
        {   
            double t = INFINITY;

            /*clog << "definiu t" << endl;*/
            bool intersectou = false;
            triangle* objeto_intersectado = search_first_intersection(r, objetos, intersectou, nullptr);
            if (objeto_intersectado == nullptr) { t = INFINITY; }
            else { t = ray_color(r, *objeto_intersectado); }

            /*clog << "depois de achar o triangulo" << endl;*/

            if(t!=INFINITY)
            {   
                index++;
                point intersection = r.f(t);
                vetor normal = objeto_intersectado->getNormal().normalizar();
                vetor objeto_color = objeto_intersectado->getColor();
                
                final_color =  final_color + (objeto_intersectado->getKa().getX() * ambient_light); // componente ambiente
                
                vetor view_dir = r.getDirection().normalizar();
                vetor view_spec = (r.getOrigin() - intersection).normalizar();
                for (const auto& light : lights) {
                    vetor light_dir = (light.getPosition() - intersection).normalizar();
                    
                    vetor reflect_dir = reflect(light_dir, normal);
                    
                    // Componente difusa
                    double diff = std::max(light_dir.produto_escalar(normal), 0.0);
                    final_color = final_color + ((objeto_intersectado->getKd().getX() * objeto_color) * diff);                            

                    // Componente especular
                    double spec = pow(std::max(view_spec.produto_escalar(reflect_dir), 0.0), objeto_intersectado->getShininess());
                    final_color = final_color + (light.getColor() * spec) * (objeto_intersectado->getKs().getX());

                }

                // Componente reflexão
                if(objeto_intersectado->getD() != 0) {   
                    vetor dir_reflec_ray = reflect(view_dir*(-1), normal);
                    point new_intersection = intersection + dir_reflec_ray * 0.00001;
                    ray reflec_ray(new_intersection, dir_reflec_ray);
                    vetor reflec_color = phong_shading_BSP(reflec_ray,objetos,lights, ambient_light, index);
                    reflec_color = reflec_color*objeto_intersectado->getD();
                    final_color = final_color + reflec_color;

                }

                
                // Componente refração
                if (objeto_intersectado->getNi() != 0) {
                    double n1 = 1; // Índice de refração do ar
                    double n2 = objeto_intersectado->getNi(); // Índice de refração do objeto
                    vetor dir_refrac_ray;
                    /*if(index==1) */
                    /*{*/
                    /*    dir_refrac_ray = refract(view_spec, normal, n1, n2);*/
                    /*    dir_refrac_ray = dir_refrac_ray * -1;*/
                    /*}*/
                    dir_refrac_ray = refract(view_dir, normal, n1, n2);
                    if (dir_refrac_ray.norma() != 0) { // Verifica se não houve reflexão total
                        point new_intersection = intersection + dir_refrac_ray * 0.00001;
                        ray refrac_ray(new_intersection, dir_refrac_ray);
                        vetor refrac_color = phong_shading_BSP(refrac_ray,objetos,lights, ambient_light, index);
                        refrac_color = refrac_color * (objeto_intersectado->getNi());
                        final_color = final_color + (refrac_color);
                    }
                }
            }
        }
        return final_color;
    }

    // calcular se o ponto está a frente, atrás ou é coplanar
    int classify_point(point p, vetor& normal, point& origem) {
        vetor dir_ponto = (p - origem).normalizar();
        double dot = dir_ponto.produto_escalar(normal.normalizar());
        if (dot == 0 || (dot < 1e-6 && dot > -1e-6)) return 0;         // coplanar
        else if (dot < 0) return -1;    // atrás do triangulo
        else return 1;                  // a frente do triangulo
    }

    // Função para encontrar o ponto de interseção entre a aresta e o plano
    point intersect_plane(point& p1, point& p2, vetor& normal, point& origem) {
        vetor dir = p2 - p1;
        double t = (origem - p1).produto_escalar(normal) / dir.produto_escalar(normal);
        return p1 + dir * t;
    }

    // separar o triangulo em relacao a um plano
    vector<triangle*> split_triangles(triangle* tri, vetor& normal, point& origem) {

        vector<triangle*> split_res;

        vetor norm = tri->getNormal();
        vetor kd = tri->getKd();
        vetor ks = tri->getKs();
        vetor ke = tri->getKe();
        vetor ka = tri->getKa();
        double ni = tri->getNi();
        double shininess = tri->getShininess();
        double D = tri->getD();

        triangle* tri1;
        triangle* tri2;
        triangle* tri3;
        triangle* tri4;

        int classificacao_1 = classify_point(tri->getA(), normal, origem);
        int classificacao_2 = classify_point(tri->getB(), normal, origem);
        int classificacao_3 = classify_point(tri->getC(), normal, origem);

        // todos os pontos estão do mesmo lado ou são coplanares
        if (classificacao_1 == classificacao_2 && classificacao_2 == classificacao_3) {
            split_res.push_back(tri);
            return split_res;
        }

        point frente[3], atras[3];
        int nf = 0, na = 0; // numero_frente e numero_atras

        if (classificacao_1 <= 0) atras[na++] = tri->getA();
        if (classificacao_1 >= 0) frente[nf++] = tri->getA();

        if (classificacao_2 <= 0) atras[na++] = tri->getB();
        if (classificacao_2 >= 0) frente[nf++] = tri->getB();

        if (classificacao_3 <= 0) atras[na++] = tri->getC();
        if (classificacao_3 >= 0) frente[nf++] = tri->getC();

        if (nf == 1 && na == 2) {
            point frente_inter1 = intersect_plane(frente[0], atras[0], normal, origem);
            point frente_inter2 = intersect_plane(frente[0], atras[1], normal, origem);

            tri1 = new triangle(norm, frente[0], frente_inter1, frente_inter2, kd, ks, ke, ka, shininess, ni, D);
            tri2 = new triangle(norm, atras[0], atras[1], frente_inter1, kd, ks, ke, ka, shininess, ni, D);
            tri3 = new triangle(norm, frente_inter1, atras[1], frente_inter2, kd, ks, ke, ka, shininess, ni, D);

            split_res.push_back(tri1);
            split_res.push_back(tri2);
            split_res.push_back(tri3);

        } else if (nf == 2 && na == 1) {
            point atras_inter1 = intersect_plane(atras[0], frente[0], normal, origem);
            point atras_inter2 = intersect_plane(atras[0], frente[1], normal, origem);

            tri1 = new triangle(norm, atras[0], atras_inter1, atras_inter2, kd, ks, ke, ka, shininess, ni, D);
            tri2 = new triangle(norm, frente[0], atras_inter1, frente[1], kd, ks, ke, ka, shininess, ni, D);
            tri3 = new triangle(norm, frente[1], atras_inter1, atras_inter2, kd, ks, ke, ka, shininess, ni, D);

            split_res.push_back(tri1);
            split_res.push_back(tri2);
            split_res.push_back(tri3);

        }
        else if (nf == 2 && na == 2) {
            point inter1 = intersect_plane(atras[0], frente[0], normal, origem);
            point inter2 = intersect_plane(atras[1], frente[1], normal, origem);

            tri1 = new triangle(norm, atras[1], inter1, frente[0], kd, ks, ke, ka, shininess, ni, D);
            tri2 = new triangle(norm, atras[0], inter1, frente[1], kd, ks, ke, ka, shininess, ni, D);
            tri3 = new triangle(norm, atras[1], inter2, frente[0], kd, ks, ke, ka, shininess, ni, D);
            tri4 = new triangle(norm, atras[0], inter2, frente[1], kd, ks, ke, ka, shininess, ni, D);

            split_res.push_back(tri1);
            split_res.push_back(tri2);
            split_res.push_back(tri3);
            split_res.push_back(tri4);

        }
        else if (nf == 3 || na == 3) split_res.push_back(tri);

    return split_res;
    }


    struct BSP_node* create_bsp(struct BSP_node* head) {

        if (head->node_triangles.size() < 2) { return head; }

        /*clog << "comecou a fazer a BSP" << endl;*/
        /*clog << head->node_triangles.size() << endl;*/

        BSP_node *atras = new BSP_node();
        BSP_node *afrente = new BSP_node();

        head->atras = atras;
        head->afrente = afrente;

        head->origem = head->node_triangles[0]->getPonto();
        head->normal = head->node_triangles[0]->getNormal();

        vector<triangle*> novos_triangulos;
        novos_triangulos.push_back(head->node_triangles[0]);

        point origem = head->origem;
        vetor normal = head->normal;

        for (int i = 1; i < head->node_triangles.size(); i++) {

            vector<triangle*> st = split_triangles(head->node_triangles[i], normal, origem);

            for (int j = 0; j < st.size(); j++) {

                int c0, c1, c2;
                c0 = classify_point(st[j]->getA(), normal, origem);
                c1 = classify_point(st[j]->getB(), normal, origem);
                c2 = classify_point(st[j]->getC(), normal, origem);

                if (c0 == -1 || c1 == -1 || c2 == -1) { /*clog << "atras" << endl;*/ atras->node_triangles.push_back(st[j]); }
                else if (c0 == 1 || c1 == 1 || c2 == 1) { /*clog << "afrente" << endl;*/ afrente->node_triangles.push_back(st[j]); }
                else  { /*clog << "coplanar" << endl;*/ novos_triangulos.push_back(st[j]); }

            }
        }

        head->node_triangles = novos_triangulos;

        if (atras->node_triangles.size() >= 2) {
            create_bsp(atras);
        }
        if (afrente->node_triangles.size() >= 2) {
            create_bsp(afrente);
        }

        return head;
    }


    void render(vector<triangle*>& triangulos, const vector<light>& lights, const vetor& ambient_light) {

        clog << "comecou render" << endl;

        // fazer BSP aqui
        BSP_node* BSP = new BSP_node();
        BSP->node_triangles = triangulos;
        BSP->normal = triangulos[0]->getNormal();
        BSP->origem = triangulos[0]->getPonto();

        BSP = create_bsp(BSP);

        clog << BSP->node_triangles.size() << endl;
        clog << BSP->atras->node_triangles.size() << endl;
        clog << BSP->afrente->node_triangles.size() << endl;

        int image_width = this->width;
        int image_height = int(image_width / aspect_ratio);
        image_height = (image_height < 1) ? 1 : image_height;
        
        double viewport_width = viewport_height * aspect_ratio;
        point camera_center = this->position;

        // Vetores que apontam as extremidades da tela
        vetor viewport_u = U * (viewport_width * -1);
        vetor viewport_v = V * (viewport_height * -1);

        // Vetores usados para calcular a posição do próximo pixel (ponto+(vetor*quantidade de pixels))
        vetor pixel_delta_u = viewport_u / image_width;
        vetor pixel_delta_v = viewport_v / image_height;

        // Ponto superior esquerdo do pixel inicial
        point viewport_upper_left = camera_center + W * focal_length - viewport_u / 2 - viewport_v / 2;

        // Calculando o ponto central do pixel
        point pixel00_loc = viewport_upper_left + (pixel_delta_u + pixel_delta_v) * 0.5;

        std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

        for (int j = 0; j < image_height; j++) {
            std::clog << "\rLinhas restantes: " << (image_height - j) << ' ' << std::flush;
            for (int i = 0; i < image_width; i++) {
                auto pixel_center = pixel00_loc + pixel_delta_u * i + pixel_delta_v * j;
                auto ray_direction = pixel_center - camera_center;
                ray_direction = ray_direction.normalizar();
                ray r(camera_center, ray_direction);
                
                // tuple<int, double, vetor> pixel_info(0, 0, vetor(0, 0, 0));

                vetor final_color = phong_shading_BSP(r,BSP,lights,ambient_light,0);
                final_color.write_color(cout);
            }
        }
        std::clog << "\rDone.                 \n";
    }

};

#endif
