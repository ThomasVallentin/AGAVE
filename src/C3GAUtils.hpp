#ifndef C3GAUTILS_H
#define C3GAUTILS_H

#include <c3gaTools.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

namespace c3ga {


enum class MvecType
{
    Unknown = 0,

    // Grade 0
    Scalar,

    // Grade 1
    Point,
    DualTangentTriVector = Point,
    DualSphere,
    ImaginaryDualSphere,
    DualPlane,

    // Grade 2
    DualLine,
    FlatPoint,
    DualCircle,
    ImaginaryPairPoint = DualCircle,
    PairPoint,
    ImaginaryDualCircle = PairPoint,
    TangentVector,
    DualTangentBiVector = TangentVector,

    // Grade 3
    Line,
    DualFlatPoint,
    Circle,
    ImaginaryDualPairPoint = Circle,
    ImaginaryCircle,
    DualPairPoint = ImaginaryCircle,
    TangentBivector,
    DualTangentVector = TangentBivector,

    // Grade 4
    DualPoint,
    TangentTrivector = DualPoint,
    Sphere,
    ImaginarySphere,
    Plane,

    // Grade 5
    PseudoScalar,

    // Misc
    NullVector,
    NonHomogenousMultiVector,
};


// This function is derived from c3ga::whoAmI() from the Garamon library.
// It simply returns enums instead of string to make it more convenient to work with.
template <typename T>
MvecType getTypeOf(Mvec<T> mv) 
{
    // zero error checking
    const T epsilon = std::numeric_limits<T>::epsilon();

    // extract grade
    std::vector<unsigned int> grades_ = mv.grades();

    // null vector
    if(grades_.size() == 0) 
        return MvecType::NullVector;

    // non-homogeneous multivector
    if(grades_.size() > 1){
        return MvecType::NonHomogenousMultiVector;
        // check if versor ...
    }

    // blade
    if(grades_.size() == 1){

        // numerical stability: scale the multivector so that the average of the coeff is 1
        auto blade = mv.findGrade(grades_[0])->vec;
        mv /= blade.array().abs().sum() / blade.size();
        //return std::to_string((double)blade.array().abs().sum()/ blade.size());

        // extract properties
        T square = (mv | mv);
        c3ga::Mvec<T> ei_outer_mv(c3ga::ei<T>() ^ mv);
        bool squareToZero = (fabs(square) <= 1.0e3*epsilon);
        bool roundObject  = !(fabs(ei_outer_mv.quadraticNorm()) < epsilon);
        //return "squareToZero : " + std::to_string(squareToZero) + " | roundObject : " +  std::to_string(roundObject) + " | ei_outer_mv.norm() : " + std::to_string(ei_outer_mv.quadraticNorm()) + " | square : " + std::to_string(fabs(square));

        switch(grades_[0]){

            ////////////////////////////////
            case 0 : // grade 0
                return MvecType::Scalar;
                break;

            ////////////////////////////////
            case 1 : // grade 1
                {
                    // point (dual tangent trivector)
                    if( squareToZero && roundObject )
                        return MvecType::Point;

                    // dual sphere
                    if( square > epsilon && roundObject )
                        return MvecType::DualSphere;

                            // imaginary dual sphere (dual sphere with negative radius)
                    if( square < -epsilon && roundObject)
                        return MvecType::ImaginaryDualSphere;

                    // dual plane   
                    if((!roundObject))
                        return MvecType::DualPlane;

                    // unknown
                    return MvecType::Unknown;
                    break;        				
                }

            ////////////////////////////////
            case 2 : // grade 2
                {
                    // tangent vector (put this test first in the grade 2 list)
                    bool smallerSquareToZero = (fabs(square) <= 1.0e3*epsilon);
                    if(smallerSquareToZero && roundObject)
                        return MvecType::TangentVector;

                    // pair point
                    if(roundObject && (square > epsilon) ) 
                        return MvecType::PairPoint;

                    // imaginary pair point
                    if(roundObject && (square < -epsilon))
                        return MvecType::DualCircle;

                    // for flat points and dual lines
                    bool onlyBivectorInfinity = (fabs( ( (mv ^ c3ga::ei<double>() ) | c3ga::e0<double>() ).quadraticNorm()) < epsilon);

                    // flat point
                    // no euclidian or eO bivector : only e_ix
                    if((!roundObject) && onlyBivectorInfinity) 
                        return MvecType::FlatPoint;

                    // dual line
                    // no origine bivector 
                    if((!roundObject) && (!onlyBivectorInfinity)) 
                        return MvecType::DualLine;

                    // unknown
                    return MvecType::Unknown;
                    break;        				
                }


            ////////////////////////////////
            case 3 : // grade 3
                {

                    // tangent bivector (put this test first in the grade 2 list)
                    bool smallerSquareToZero = (fabs(square) <= 1.0e3*epsilon);
                    if(smallerSquareToZero && roundObject)
                        return MvecType::TangentBivector;

                    // circle
                    if(roundObject && (square > epsilon) ) 
                        return MvecType::Circle;

                    // imaginary circle
                    if(roundObject && (square < -epsilon))
                        return MvecType::ImaginaryCircle;


                    // for dual flat points and lines
                    bool onlyTrivectorInfinity = (fabs( ( (mv ^ c3ga::ei<double>() ) | c3ga::e0<double>() ).quadraticNorm()) < epsilon);

                    // dual flat point
                    // no e0 trivector
                    if((!roundObject) && !onlyTrivectorInfinity) 
                        return MvecType::DualFlatPoint;

                    // line
                    // only e_ixx trivectors
                    if((!roundObject) && onlyTrivectorInfinity) 
                        return MvecType::Line;


                    // unknown
                    return MvecType::Unknown;
                    break;        				
                }


            ////////////////////////////////
            case 4 : // grade 4
                {
                    // dual euclidean vector (removed because a plane can also have this form)
                    // if(fabs(mv[c3ga::E0123]) < epsilon && fabs(mv[c3ga::E123i]) < epsilon)
                    // 	return "dual Euclidean vector";

                    // dual point
                    if( squareToZero && roundObject )
                        return MvecType::DualPoint;

                    // sphere and imaginary sphere: compute radius from dual 
                    c3ga::Mvec<double> dualSphere = mv.dual();
                    T dualSquare = dualSphere | dualSphere;

                    // sphere
                    if( dualSquare > epsilon && roundObject )
                        return MvecType::Sphere;

                    // imaginary sphere (sphere with negative radius)
                    if( dualSquare < -epsilon && roundObject)
                        return MvecType::ImaginarySphere;

                    // plane   
                    if((!roundObject))
                        return MvecType::Plane;

                    // unknown
                    return MvecType::Unknown;
                    break;        				
                }


            ////////////////////////////////
            case 5 : // grade 5
                {
                    // unknown
                    return MvecType::PseudoScalar;
                    break;        				
                }

            ////////////////////////////////
            default : return MvecType::Unknown;

        }
    }

    return MvecType::Unknown;
}

template <typename T>
glm::mat4 extractDualCircleMatrix(const Mvec<T>& dualCircle)
{
    T radius;
    Mvec<T> center, direction;
    extractDualCircle(dualCircle, radius, center, direction);
    std::cout << whoAmI(dualCircle) << std::endl; 

    glm::mat4 t = glm::translate(glm::mat4(1.0f), {center[E1], center[E2], center[E3]});
    glm::mat4 r = glm::toMat4(glm::quat{glm::vec3(0, 1, 0), glm::vec3(direction[E1], direction[E2], direction[E3])});
    glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(radius));

    return t * r * s;
}

template <typename T>
glm::mat4 extractDualSphereMatrix(const Mvec<T>& dualSphere)
{
    T radius;
    Mvec<T> center;
    radiusAndCenterFromDualSphere(dualSphere, radius, center);
    std::cout << whoAmI(dualSphere) << std::endl; 

    glm::mat4 t = glm::translate(glm::mat4(1.0f), {center[E1], center[E2], center[E3]});
    glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(std::abs(radius)));

    std::cout << glm::to_string(t * s) << std::endl;
    return t * s;
}

} // namespace c3ga



#endif // C3GAUTILS_H