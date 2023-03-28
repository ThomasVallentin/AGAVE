#include "Samples.hpp"


namespace Samples {


void LoadSpheresIntersect(const LayerStackPtr& layerStack)
{
    if (!layerStack)
        return;
    
    layerStack->Clear();
    
    MvecArray objects = {c3ga::dualSphere<double>(0, 0, 0, 1).dual()};
    auto lyr1 = layerStack->NewLayer("Layer1", objects);
    objects = {c3ga::dualSphere<double>(0, 1, 0, 1).dual()};
    auto lyr2 = layerStack->NewLayer("Layer2", objects);
    auto lyr3 = layerStack->NewCombination("Layer3", lyr1, lyr2, Operators::OuterProduct);
    lyr3->SetDual(true);
    lyr3->SetSourceDual(0, true);
    lyr3->SetSourceDual(1, true);
}

void LoadPlanesIntersect(const LayerStackPtr& layerStack)
{
    if (!layerStack)
        return;
    
    layerStack->Clear();
    
    MvecArray objects = {c3ga::point<double>(2, 2, 0) ^
                         c3ga::point<double>(1, 2, 1) ^
                         c3ga::point<double>(3, 2, 4) ^
                         c3ga::ei<double>(),
                         c3ga::point<double>(2, 2, 0) ^
                         c3ga::point<double>(2, 1, 1) ^
                         c3ga::point<double>(2, 3, 4) ^
                         c3ga::ei<double>()};
    auto planes = layerStack->NewLayer("Planes", objects);
    auto intersect = layerStack->NewSelfCombination("Planes intersection", planes, 2);
    intersect->SetSourceDual(0, true);
    intersect->SetDual(true);
}

void LoadCompleteExample(const LayerStackPtr& layerStack)
{
    if (!layerStack)
        return;
    
    layerStack->Clear();
    
    // Objects layers
    MvecArray objects = {c3ga::dualSphere<double>(2, 0.5, 2, 1).dual()};
    auto sphere1 = layerStack->NewLayer("Sphere 1", objects);

    objects = {c3ga::dualSphere<double>(2, 1.5, 2, 1).dual()};
    auto sphere2 = layerStack->NewLayer("Sphere 2", objects);

    objects = {c3ga::point<double>(2, 0, 0) ^
               c3ga::point<double>(1, 0, 1) ^
               c3ga::point<double>(3, 0, 4) ^
               c3ga::ei<double>()};
    auto plane1 = layerStack->NewLayer("Plane 1", objects);

    objects = {c3ga::point<double>(2, -2, 0) ^
               c3ga::point<double>(1, -1, 1) ^
               c3ga::point<double>(2, -2, 4) ^
               c3ga::ei<double>()};
    auto plane2 = layerStack->NewLayer("Plane 2", objects);

    // Intersections

    auto spheresIntersect = layerStack->NewCombination("Spheres intersection", sphere1, sphere2, Operators::OuterProduct);
    spheresIntersect->SetDual(true);
    spheresIntersect->SetSourceDual(0, true);
    spheresIntersect->SetSourceDual(1, true);
    
    auto planesIntersect = layerStack->NewCombination("Planes intersection", plane1, plane2, Operators::OuterProduct);
    planesIntersect->SetDual(true);
    planesIntersect->SetSourceDual(0, true);
    planesIntersect->SetSourceDual(1, true);

    auto spherePlaneIntersect = layerStack->NewCombination("Sphere plane intersection", sphere1, plane1, Operators::OuterProduct);
    spherePlaneIntersect->SetDual(true);
    spherePlaneIntersect->SetSourceDual(0, true);
    spherePlaneIntersect->SetSourceDual(1, true);
}


void LoadSynthWaveExample(const LayerStackPtr& layerStack)
{
    if (!layerStack)
        return;
    
    layerStack->Clear();

    auto spheres = layerStack->NewRandomGenerator("RandomSpheres", c3ga::MvecType::Sphere, 25, 5.0);
    std::dynamic_pointer_cast<Explicit>(spheres->GetProvider())->SetAnimated(true);
    spheres->SetVisible(false);

    MvecArray objects;
    for (double y=-1 ; y < 1 ; y += 0.2)
    {
        objects.push_back(c3ga::point<double>(2, y, 0) ^
                          c3ga::point<double>(1, y, 1) ^
                          c3ga::point<double>(3, y, 4) ^
                          c3ga::ei<double>());
    }

    auto planes = layerStack->NewLayer("Planes", objects);
    planes->SetVisible(false);

    auto circles = layerStack->NewCombination("Circles", spheres, planes, Operators::OuterProduct);
    circles->SetDual(true);
    circles->SetSourceDual(0, true);
    circles->SetSourceDual(1, true);
    circles->SetVisible(false);

    objects = {c3ga::point<double>(2, -2, 0) ^
               c3ga::point<double>(1, -1, 1) ^
               c3ga::point<double>(2, -2, 4) ^
               c3ga::ei<double>()};
    auto plane2 = layerStack->NewLayer("Plane_2", objects);
    plane2->SetVisible(false);

    auto pairPoints = layerStack->NewCombination("PairPoints", circles, plane2, Operators::OuterProduct);
    pairPoints->SetDual(true);
    pairPoints->SetSourceDual(0, true);
    pairPoints->SetSourceDual(1, true);

    auto lines = layerStack->NewCombination("PairPoints", planes, plane2, Operators::OuterProduct);
    lines->SetDual(true);
    lines->SetSourceDual(0, true);
    lines->SetSourceDual(1, true);
}


}