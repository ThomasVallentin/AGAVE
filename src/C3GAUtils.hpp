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


template <typename T>
Mvec<T> vector(const T& x, const T& y, const T& z)
{		
    Mvec<T> result;
    
    result[E1] = x;
    result[E2] = y;
    result[E3] = z;
    result[Ei] = 0.0;
    result[E0] = 0.0;

    return result;
}

template <typename T>
Mvec<T> randomVector()
{		
    std::uniform_real_distribution<T> distrib(-1.0, 1.0);
    return vector(distrib(generator), distrib(generator), distrib(generator));
}

template <typename T>
Mvec<T> translator(const Mvec<T>& translation)
{		
    return 1.0 - 0.5 * translation * ei<T>();
}


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
        Mvec<T> ei_outer_mv(ei<T>() ^ mv);
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
                    bool onlyBivectorInfinity = (fabs( ( (mv ^ ei<double>() ) | e0<double>() ).quadraticNorm()) < epsilon);

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
                    bool onlyTrivectorInfinity = (fabs( ( (mv ^ ei<double>() ) | e0<double>() ).quadraticNorm()) < epsilon);

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
                    Mvec<double> dualSphere = mv.dual();
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

inline std::string typeToName(const MvecType& type)
{
    switch (type)
    {
        case MvecType::Scalar:
            return "Scalar";

        case MvecType::Point:
            return"Point";

        case MvecType::DualSphere:
            return "DualSphere";

        case MvecType::ImaginaryDualSphere:
            return "ImaginaryDualSphere";

        case MvecType::DualPlane:
            return "DualPlane";

        case MvecType::DualLine:
            return "DualLine";

        case MvecType::FlatPoint:
            return "FlatPoint";

        case MvecType::DualCircle:
            return "DualCircle";

        case MvecType::PairPoint:
            return "PairPoint";

        case MvecType::TangentVector:
            return "TangentVector";

        case MvecType::Line:
            return "Line";
            
        case MvecType::DualFlatPoint:
            return "DualFlatPoint";

        case MvecType::Circle:
            return "Circle";

        case MvecType::ImaginaryCircle:
            return "ImaginaryCircle";

        case MvecType::TangentBivector:
            return "TangentBivector";

        case MvecType::DualPoint:
            return "DualPoint";

        case MvecType::Sphere:
            return "Sphere";

        case MvecType::ImaginarySphere:
            return "ImaginarySphere";

        case MvecType::Plane:
            return"Plane";

        case MvecType::PseudoScalar:
            return "PseudoScalar";

        case MvecType::NullVector:
            return "NullVector";

        case MvecType::NonHomogenousMultiVector:
            return "NonHomogenousMultiVector";

        default:
            return "Unknown";
    }
}

// Convert an mvector from a type to another one
// Currently supports point, sphere and dualSphere conversion
template <typename T>
Mvec<T> convert(const Mvec<T>& mv, const MvecType& fromType, const MvecType& toType)
{
    Mvec<T> result;

    // Returns as soon as possible if we cannot generate the requested MvecType
    switch (toType)
    {
        case MvecType::Point: 
        case MvecType::Sphere: 
        case MvecType::DualSphere: {
            break;
        }

        default:
            return result;
    }

    Mvec<T> center, direction;
    T radius;
    switch (fromType)
    {
        case MvecType::Point: {
            center = mv;
            radius = 1.0;
            break;
        }

        case MvecType::Sphere: 
        case MvecType::ImaginarySphere: {
            radiusAndCenterFromDualSphere(mv.dual(), radius, center);
            break;
        }
        case MvecType::DualSphere: 
        case MvecType::ImaginaryDualSphere: {
            radiusAndCenterFromDualSphere(mv, radius, center);
            break;
        }

        case MvecType::Circle: 
        case MvecType::ImaginaryCircle: {
            extractDualCircle(mv.dual(), radius, center, direction);
            break;
        }
        case MvecType::DualCircle: 
        case MvecType::ImaginaryDualCircle: {
            extractDualCircle(mv, radius, center, direction);
            break;
        }

        default:
            return result;
    }

    switch (toType)
    {
        case MvecType::Point: {
            return point(center[E1], center[E2], center[E3]);
        }
        case MvecType::Sphere: {
            return dualSphere<T>(center[E1], center[E2], center[E3], radius).dual();
        }
        case MvecType::DualSphere: {
            return dualSphere<T>(center[E1], center[E2], center[E3], radius);
        }

        default:
            return result;
    }
}

// Convenience overloading
template <typename T>
Mvec<T> convert(const Mvec<T>& mv, const MvecType& toType)
{
    return convert(mv, toType, getTypeOf(mv));
}


// == Feature extraction functions ==

template <typename T>
void originAndDirectionFromDualLine(const Mvec<T>& dualLine, Mvec<T>& origin, Mvec<T>& direction)
{
    direction[E1] =  dualLine[E23];
    direction[E2] = -dualLine[E13];
    direction[E3] =  dualLine[E12];
    origin[E1]    =  dualLine[E1i];
    origin[E2]    =  dualLine[E2i];
    origin[E3]    =  dualLine[E3i];
    origin = (direction ^ origin) / (direction | direction);
    origin = point(origin[E23], -origin[E13], origin[E12]);
}


// == Matrix extraction functions ==

template <typename T>
glm::mat4 extractDualCircleMatrix(const Mvec<T>& dualCircle)
{
    T radius;
    Mvec<T> center, direction;
    extractDualCircle(dualCircle, radius, center, direction);

    glm::mat4 t = glm::translate(glm::mat4(1.0f), {center[E1], center[E2], center[E3]});
    glm::mat4 r = glm::toMat4(glm::quat{glm::vec3(0, 0, 1), glm::vec3(direction[E1], direction[E2], direction[E3])});
    glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(radius));

    return t * r * s;
}

template <typename T>
glm::mat4 extractDualSphereMatrix(const Mvec<T>& dualSphere)
{
    T radius;
    Mvec<T> center;
    radiusAndCenterFromDualSphere(dualSphere, radius, center);

    glm::mat4 t = glm::translate(glm::mat4(1.0f), {center[E1], center[E2], center[E3]});
    glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(radius));
    return t * s;
}

template <typename T>
glm::mat4 extractDualLineMatrix(const Mvec<T>& dualLine)
{
    Mvec<T> origin, direction;
    originAndDirectionFromDualLine(dualLine, origin, direction);

    glm::mat4 t = glm::translate(glm::mat4(1.0f), {origin[E1], origin[E2], origin[E3]});
    glm::mat4 r = glm::toMat4(glm::quat{glm::vec3(0, 0, 1), glm::vec3(direction[E1], direction[E2], direction[E3])});

    return t * r;
}

template <typename T>
glm::mat4 extractDualPlaneMatrix(const Mvec<T>& dualPlane)
{
    T a = dualPlane[E1], b = dualPlane[E2], c = dualPlane[E3], d = dualPlane[Ei];
    glm::vec3 normal = glm::normalize(glm::vec3(a, b, c));
    glm::mat4 r = glm::toMat4(glm::quat{glm::vec3(0, 1, 0), normal});
    
    // We pin the plane to x=0 and z=0. In that way ax+by+cz+d=0 gives y=d/b
    float y = b != 0 ? (d / b) : 0.0f;

    return glm::translate(glm::mat4(1.0f), {0.0f, y, 0.0f}) * r;
}




} // namespace c3ga


#endif // C3GAUTILS_H