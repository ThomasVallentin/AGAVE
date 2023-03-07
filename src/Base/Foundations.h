#include <memory>
#include <filesystem>

namespace fs=std::filesystem;

#define DECLARE_PTR_TYPE(_type) typedef std::shared_ptr<_type> _type##Ptr; \
                                typedef std::weak_ptr<_type> _type##WeakPtr
#define DECLARE_CONST_PTR_TYPE(_type) typedef std::shared_ptr<const _type> _type##ConstPtr; \
                                      typedef std::weak_ptr<const _type> _type##ConstWeakPtr

