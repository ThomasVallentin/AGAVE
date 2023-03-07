#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include <string>

// From boost::hash_combine
template <class T>
void HashCombine(size_t& seed, const T& value)
{
  std::hash<T> hash;
  seed ^= hash(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}


#endif // TYPEUTILS_H
